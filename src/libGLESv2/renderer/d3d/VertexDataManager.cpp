#include "precompiled.h"
//
// Copyright (c) 2002-2012 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// VertexDataManager.h: Defines the VertexDataManager, a class that
// runs the Buffer translation process.

#include "libGLESv2/renderer/d3d/VertexDataManager.h"
#include "libGLESv2/renderer/d3d/BufferD3D.h"

#include "libGLESv2/Buffer.h"
#include "libGLESv2/ProgramBinary.h"
#include "libGLESv2/VertexAttribute.h"
#include "libGLESv2/renderer/d3d/VertexBuffer.h"
#include "libGLESv2/renderer/Renderer.h"

namespace
{
    enum { INITIAL_STREAM_BUFFER_SIZE = 1024*1024 };
    // This has to be at least 4k or else it fails on ATI cards.
    enum { CONSTANT_VERTEX_BUFFER_SIZE = 4096 };
}

namespace rx
{

static int ElementsInBuffer(const gl::VertexAttribute &attrib, unsigned int size)
{
    // Size cannot be larger than a GLsizei
    if (size > static_cast<unsigned int>(std::numeric_limits<int>::max()))
    {
        size = static_cast<unsigned int>(std::numeric_limits<int>::max());
    }

    GLsizei stride = ComputeVertexAttributeStride(attrib);
    return (size - attrib.offset % stride + (stride - ComputeVertexAttributeTypeSize(attrib))) / stride;
}

static int StreamingBufferElementCount(const gl::VertexAttribute &attrib, int vertexDrawCount, int instanceDrawCount)
{
    // For instanced rendering, we draw "instanceDrawCount" sets of "vertexDrawCount" vertices.
    //
    // A vertex attribute with a positive divisor loads one instanced vertex for every set of
    // non-instanced vertices, and the instanced vertex index advances once every "mDivisor" instances.
    if (instanceDrawCount > 0 && attrib.divisor > 0)
    {
        return instanceDrawCount / attrib.divisor;
    }

    return vertexDrawCount;
}

VertexDataManager::VertexDataManager(Renderer *renderer) : mRenderer(renderer)
{
    for (int i = 0; i < gl::MAX_VERTEX_ATTRIBS; i++)
    {
        mCurrentValue[i].FloatValues[0] = std::numeric_limits<float>::quiet_NaN();
        mCurrentValue[i].FloatValues[1] = std::numeric_limits<float>::quiet_NaN();
        mCurrentValue[i].FloatValues[2] = std::numeric_limits<float>::quiet_NaN();
        mCurrentValue[i].FloatValues[3] = std::numeric_limits<float>::quiet_NaN();
        mCurrentValue[i].Type = GL_FLOAT;
        mCurrentValueBuffer[i] = NULL;
        mCurrentValueOffsets[i] = 0;
    }

    mStreamingBuffer = new StreamingVertexBufferInterface(renderer, INITIAL_STREAM_BUFFER_SIZE);

    if (!mStreamingBuffer)
    {
        ERR("Failed to allocate the streaming vertex buffer.");
    }
}

VertexDataManager::~VertexDataManager()
{
    delete mStreamingBuffer;

    for (int i = 0; i < gl::MAX_VERTEX_ATTRIBS; i++)
    {
        delete mCurrentValueBuffer[i];
    }
}

GLenum VertexDataManager::prepareVertexData(const gl::VertexAttribute attribs[], const gl::VertexAttribCurrentValueData currentValues[],
                                            gl::ProgramBinary *programBinary, GLint start, GLsizei count, TranslatedAttribute *translated, GLsizei instances)
{
    if (!mStreamingBuffer)
    {
        return GL_OUT_OF_MEMORY;
    }

    for (int attributeIndex = 0; attributeIndex < gl::MAX_VERTEX_ATTRIBS; attributeIndex++)
    {
        translated[attributeIndex].active = (programBinary->getSemanticIndex(attributeIndex) != -1);
    }

    // Invalidate static buffers that don't contain matching attributes
    for (int i = 0; i < gl::MAX_VERTEX_ATTRIBS; i++)
    {
        if (translated[i].active && attribs[i].enabled)
        {
            gl::Buffer *buffer = attribs[i].buffer.get();

            if (buffer)
            {
                BufferD3D *bufferImpl = BufferD3D::makeBufferD3D(buffer->getImplementation());
                StaticVertexBufferInterface *staticBuffer = bufferImpl->getStaticVertexBuffer();

                if (staticBuffer && staticBuffer->getBufferSize() > 0 && !staticBuffer->lookupAttribute(attribs[i], NULL) &&
                    !staticBuffer->directStoragePossible(attribs[i], currentValues[i]))
                {
                    bufferImpl->invalidateStaticData();
                }
            }
        }
    }

    // Reserve the required space in the buffers
    for (int i = 0; i < gl::MAX_VERTEX_ATTRIBS; i++)
    {
        if (translated[i].active && attribs[i].enabled)
        {
            gl::Buffer *buffer = attribs[i].buffer.get();
            BufferD3D *bufferImpl = buffer ? BufferD3D::makeBufferD3D(buffer->getImplementation()) : NULL;
            StaticVertexBufferInterface *staticBuffer = bufferImpl ? bufferImpl->getStaticVertexBuffer() : NULL;
            VertexBufferInterface *vertexBuffer = staticBuffer ? staticBuffer : static_cast<VertexBufferInterface*>(mStreamingBuffer);

            if (!vertexBuffer->directStoragePossible(attribs[i], currentValues[i]))
            {
                if (staticBuffer)
                {
                    if (staticBuffer->getBufferSize() == 0)
                    {
                        int totalCount = ElementsInBuffer(attribs[i], bufferImpl->getSize());
                        if (!staticBuffer->reserveVertexSpace(attribs[i], totalCount, 0))
                        {
                            return GL_OUT_OF_MEMORY;
                        }
                    }
                }
                else
                {
                    int totalCount = StreamingBufferElementCount(attribs[i], count, instances);

                    // [OpenGL ES 3.0.2] section 2.9.4 page 40:
                    // We can return INVALID_OPERATION if our vertex attribute does not have enough backing data.
                    if (bufferImpl && ElementsInBuffer(attribs[i], bufferImpl->getSize()) < totalCount)
                    {
                        return GL_INVALID_OPERATION;
                    }

                    if (!mStreamingBuffer->reserveVertexSpace(attribs[i], totalCount, instances))
                    {
                        return GL_OUT_OF_MEMORY;
                    }
                }
            }
        }
    }

    // Perform the vertex data translations
    for (int i = 0; i < gl::MAX_VERTEX_ATTRIBS; i++)
    {
        if (translated[i].active)
        {
            GLenum result;

            if (attribs[i].enabled)
            {
                result = storeAttribute(attribs[i], currentValues[i], &translated[i],
                                        start, count, instances);
            }
            else
            {
                if (!mCurrentValueBuffer[i])
                {
                    mCurrentValueBuffer[i] = new StreamingVertexBufferInterface(mRenderer, CONSTANT_VERTEX_BUFFER_SIZE);
                }

                result = storeCurrentValue(attribs[i], currentValues[i], &translated[i],
                                           &mCurrentValue[i], &mCurrentValueOffsets[i],
                                           mCurrentValueBuffer[i]);
            }

            if (result != GL_NO_ERROR)
            {
                return result;
            }
        }
    }

    for (int i = 0; i < gl::MAX_VERTEX_ATTRIBS; i++)
    {
        if (translated[i].active && attribs[i].enabled)
        {
            gl::Buffer *buffer = attribs[i].buffer.get();

            if (buffer)
            {
                BufferD3D *bufferImpl = BufferD3D::makeBufferD3D(buffer->getImplementation());
                bufferImpl->promoteStaticUsage(count * ComputeVertexAttributeTypeSize(attribs[i]));
            }
        }
    }

    return GL_NO_ERROR;
}

GLenum VertexDataManager::storeAttribute(const gl::VertexAttribute &attrib,
                                         const gl::VertexAttribCurrentValueData &currentValue,
                                         TranslatedAttribute *translated,
                                         GLint start,
                                         GLsizei count,
                                         GLsizei instances)
{
    gl::Buffer *buffer = attrib.buffer.get();

    if (!buffer && attrib.pointer == NULL)
    {
        // This is an application error that would normally result in a crash, but we catch it and return an error
        ERR("An enabled vertex array has no buffer and no pointer.");
        return GL_INVALID_OPERATION;
    }

    BufferD3D *storage = buffer ? BufferD3D::makeBufferD3D(buffer->getImplementation()) : NULL;
    StaticVertexBufferInterface *staticBuffer = storage ? storage->getStaticVertexBuffer() : NULL;
    VertexBufferInterface *vertexBuffer = staticBuffer ? staticBuffer : static_cast<VertexBufferInterface*>(mStreamingBuffer);
    bool directStorage = vertexBuffer->directStoragePossible(attrib, currentValue);

    unsigned int streamOffset = 0;
    unsigned int outputElementSize = 0;

    if (directStorage)
    {
        outputElementSize = ComputeVertexAttributeStride(attrib);
        streamOffset = attrib.offset + outputElementSize * start;
    }
    else if (staticBuffer)
    {
        if (!staticBuffer->getVertexBuffer()->getSpaceRequired(attrib, 1, 0, &outputElementSize))
        {
            return GL_OUT_OF_MEMORY;
        }

        if (!staticBuffer->lookupAttribute(attrib, &streamOffset))
        {
            // Convert the entire buffer
            int totalCount = ElementsInBuffer(attrib, storage->getSize());
            int startIndex = attrib.offset / ComputeVertexAttributeStride(attrib);

            if (!staticBuffer->storeVertexAttributes(attrib, currentValue, -startIndex, totalCount,
                0, &streamOffset))
            {
                return GL_OUT_OF_MEMORY;
            }
        }

        unsigned int firstElementOffset = (attrib.offset / ComputeVertexAttributeStride(attrib)) * outputElementSize;
        unsigned int startOffset = (instances == 0 || attrib.divisor == 0) ? start * outputElementSize : 0;
        if (streamOffset + firstElementOffset + startOffset < streamOffset)
        {
            return GL_OUT_OF_MEMORY;
        }

        streamOffset += firstElementOffset + startOffset;
    }
    else
    {
        int totalCount = StreamingBufferElementCount(attrib, count, instances);
        if (!mStreamingBuffer->getVertexBuffer()->getSpaceRequired(attrib, 1, 0, &outputElementSize) ||
            !mStreamingBuffer->storeVertexAttributes(attrib, currentValue, start, totalCount, instances,
            &streamOffset))
        {
            return GL_OUT_OF_MEMORY;
        }
    }

    translated->storage = directStorage ? storage : NULL;
    translated->vertexBuffer = vertexBuffer->getVertexBuffer();
    translated->serial = directStorage ? storage->getSerial() : vertexBuffer->getSerial();
    translated->divisor = attrib.divisor;

    translated->attribute = &attrib;
    translated->currentValueType = currentValue.Type;
    translated->stride = outputElementSize;
    translated->offset = streamOffset;

    return GL_NO_ERROR;
}

GLenum VertexDataManager::storeCurrentValue(const gl::VertexAttribute &attrib,
                                            const gl::VertexAttribCurrentValueData &currentValue,
                                            TranslatedAttribute *translated,
                                            gl::VertexAttribCurrentValueData *cachedValue,
                                            size_t *cachedOffset,
                                            StreamingVertexBufferInterface *buffer)
{
    if (*cachedValue != currentValue)
    {
        if (!buffer->reserveVertexSpace(attrib, 1, 0))
        {
            return GL_OUT_OF_MEMORY;
        }

        unsigned int streamOffset;
        if (!buffer->storeVertexAttributes(attrib, currentValue, 0, 1, 0, &streamOffset))
        {
            return GL_OUT_OF_MEMORY;
        }

        *cachedValue = currentValue;
        *cachedOffset = streamOffset;
    }

    translated->storage = NULL;
    translated->vertexBuffer = buffer->getVertexBuffer();
    translated->serial = buffer->getSerial();
    translated->divisor = 0;

    translated->attribute = &attrib;
    translated->currentValueType = currentValue.Type;
    translated->stride = 0;
    translated->offset = *cachedOffset;

    return GL_NO_ERROR;
}

}
