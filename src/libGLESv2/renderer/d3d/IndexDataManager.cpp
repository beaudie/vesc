#include "precompiled.h"
//
// Copyright (c) 2002-2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// IndexDataManager.cpp: Defines the IndexDataManager, a class that
// runs the Buffer translation process for index buffers.

#include "libGLESv2/renderer/d3d/IndexDataManager.h"
#include "libGLESv2/renderer/d3d/BufferD3D.h"

#include "libGLESv2/Buffer.h"
#include "libGLESv2/main.h"
#include "libGLESv2/formatutils.h"
#include "libGLESv2/renderer/d3d/IndexBuffer.h"
#include "libGLESv2/renderer/Renderer.h"

namespace rx
{

IndexDataManager::IndexDataManager(Renderer *renderer)
    : mRenderer(renderer),
      mStreamingBufferShort(NULL),
      mStreamingBufferInt(NULL)
{
}

IndexDataManager::~IndexDataManager()
{
    delete mStreamingBufferShort;
    delete mStreamingBufferInt;
}

static void convertIndices(GLenum sourceType, GLenum destinationType, const void *input, GLsizei count, void *output)
{
    if (sourceType == GL_UNSIGNED_BYTE)
    {
        ASSERT(destinationType == GL_UNSIGNED_SHORT);
        const GLubyte *in = static_cast<const GLubyte*>(input);
        GLushort *out = static_cast<GLushort*>(output);

        for (GLsizei i = 0; i < count; i++)
        {
            out[i] = in[i];
        }
    }
    else if (sourceType == GL_UNSIGNED_INT)
    {
        ASSERT(destinationType == GL_UNSIGNED_INT);
        memcpy(output, input, count * sizeof(GLuint));
    }
    else if (sourceType == GL_UNSIGNED_SHORT)
    {
        if (destinationType == GL_UNSIGNED_SHORT)
        {
            memcpy(output, input, count * sizeof(GLushort));
        }
        else if (destinationType == GL_UNSIGNED_INT)
        {
            const GLushort *in = static_cast<const GLushort*>(input);
            GLuint *out = static_cast<GLuint*>(output);

            for (GLsizei i = 0; i < count; i++)
            {
                out[i] = in[i];
            }
        }
        else UNREACHABLE();
    }
    else UNREACHABLE();
}

template <class IndexType>
static RangeUI computeRange(const IndexType *indices, GLsizei count)
{
    unsigned int minIndex = indices[0];
    unsigned int maxIndex = indices[0];

    for (GLsizei i = 1; i < count; i++)
    {
        if (minIndex > indices[i]) minIndex = indices[i];
        if (maxIndex < indices[i]) maxIndex = indices[i];
    }

    return RangeUI(minIndex, maxIndex);
}

static RangeUI computeRange(GLenum type, const GLvoid *indices, GLsizei count)
{
    switch (type)
    {
      case GL_UNSIGNED_BYTE:
        return computeRange(static_cast<const GLubyte*>(indices), count);
      case GL_UNSIGNED_INT:
        return computeRange(static_cast<const GLuint*>(indices), count);
      case GL_UNSIGNED_SHORT:
        return computeRange(static_cast<const GLushort*>(indices), count);
      default:
        UNREACHABLE();
        return RangeUI();
    }
}

gl::Error IndexDataManager::prepareIndexData(GLenum type, GLsizei count, gl::Buffer *buffer, const GLvoid *indices, TranslatedIndexData *translated)
{
    const gl::Type &typeInfo = gl::GetTypeInfo(type);

    GLenum destinationIndexType = (type == GL_UNSIGNED_INT) ? GL_UNSIGNED_INT : GL_UNSIGNED_SHORT;

    unsigned int offset = 0;
    bool alignedOffset = false;

    BufferD3D *storage = NULL;

    if (buffer != NULL)
    {
        if (reinterpret_cast<uintptr_t>(indices) > std::numeric_limits<unsigned int>::max())
        {
            return gl::Error(GL_OUT_OF_MEMORY, "Unable to support an index pointer greater than the maximum unsigned integer.");
        }
        offset = static_cast<unsigned int>(reinterpret_cast<uintptr_t>(indices));

        storage = BufferD3D::makeBufferD3D(buffer->getImplementation());

        switch (type)
        {
          case GL_UNSIGNED_BYTE:  alignedOffset = (offset % sizeof(GLubyte) == 0);  break;
          case GL_UNSIGNED_SHORT: alignedOffset = (offset % sizeof(GLushort) == 0); break;
          case GL_UNSIGNED_INT:   alignedOffset = (offset % sizeof(GLuint) == 0);   break;
          default: UNREACHABLE(); alignedOffset = false;
        }

        // check for integer overflows
        if (static_cast<unsigned int>(count) > (std::numeric_limits<unsigned int>::max() / typeInfo.bytes) ||
            typeInfo.bytes * static_cast<unsigned int>(count) + offset < offset)
        {
            return gl::Error(GL_OUT_OF_MEMORY, "Maximum index location is too large and would result in an overflow.");
        }

        if (typeInfo.bytes * static_cast<unsigned int>(count) + offset > storage->getSize())
        {
            return gl::Error(GL_OUT_OF_MEMORY, "Maximum index location is too large and would result in an overflow.");
        }

        indices = static_cast<const GLubyte*>(storage->getData()) + offset;
    }

    StaticIndexBufferInterface *staticBuffer = storage ? storage->getStaticIndexBuffer() : NULL;
    IndexBufferInterface *indexBuffer = NULL;
    bool directStorage = alignedOffset && storage && storage->supportsDirectBinding() &&
                         destinationIndexType == type;
    unsigned int streamOffset = 0;

    if (directStorage)
    {
        streamOffset = offset;

        if (!buffer->getIndexRangeCache()->findRange(type, offset, count, &translated->indexRange, NULL))
        {
            translated->indexRange = computeRange(type, indices, count);
            buffer->getIndexRangeCache()->addRange(type, offset, count, translated->indexRange, offset);
        }
    }
    else if (staticBuffer && staticBuffer->getBufferSize() != 0 && staticBuffer->getIndexType() == type && alignedOffset)
    {
        indexBuffer = staticBuffer;

        if (!staticBuffer->getIndexRangeCache()->findRange(type, offset, count, &translated->indexRange, &streamOffset))
        {
            streamOffset = (offset / typeInfo.bytes) * gl::GetTypeInfo(destinationIndexType).bytes;
            translated->indexRange = computeRange(type, indices, count);
            staticBuffer->getIndexRangeCache()->addRange(type, offset, count, translated->indexRange, streamOffset);
        }
    }
    else
    {
        translated->indexRange = computeRange(type, indices, count);
    }

    // Avoid D3D11's primitive restart index value
    // see http://msdn.microsoft.com/en-us/library/windows/desktop/bb205124(v=vs.85).aspx
    if (translated->indexRange.end == 0xFFFF && type == GL_UNSIGNED_SHORT && mRenderer->getMajorShaderModel() > 3)
    {
        destinationIndexType = GL_UNSIGNED_INT;
        directStorage = false;
        indexBuffer = NULL;
    }

    const gl::Type &destTypeInfo = gl::GetTypeInfo(destinationIndexType);

    if (!directStorage && !indexBuffer)
    {
        if (destinationIndexType == GL_UNSIGNED_INT)
        {
            if (!mStreamingBufferInt)
            {
                mStreamingBufferInt = new StreamingIndexBufferInterface(mRenderer);
                gl::Error error = mStreamingBufferInt->reserveBufferSpace(INITIAL_INDEX_BUFFER_SIZE, GL_UNSIGNED_INT);
                if (error.isError())
                {
                    return error;
                }
            }
            indexBuffer = mStreamingBufferInt;
        }
        else
        {
            ASSERT(destinationIndexType == GL_UNSIGNED_SHORT);

            if (!mStreamingBufferShort)
            {
                mStreamingBufferShort = new StreamingIndexBufferInterface(mRenderer);
                gl::Error error = mStreamingBufferShort->reserveBufferSpace(INITIAL_INDEX_BUFFER_SIZE, GL_UNSIGNED_SHORT);
                if (error.isError())
                {
                    return error;
                }
            }
            indexBuffer = mStreamingBufferShort;
        }

        unsigned int convertCount = count;

        if (staticBuffer)
        {
            if (staticBuffer->getBufferSize() == 0 && alignedOffset)
            {
                indexBuffer = staticBuffer;
                convertCount = storage->getSize() / typeInfo.bytes;
            }
            else
            {
                storage->invalidateStaticData();
                staticBuffer = NULL;
            }
        }

        if (!indexBuffer)
        {
            return gl::Error(GL_INVALID_OPERATION, "No valid index data available.");
        }

        if (convertCount > std::numeric_limits<unsigned int>::max() / destTypeInfo.bytes)
        {
            return gl::Error(GL_OUT_OF_MEMORY, "Reserving %u indices of %u bytes each exceeds the maximum buffer size.",
                             convertCount, destTypeInfo.bytes);
        }

        unsigned int bufferSizeRequired = convertCount * destTypeInfo.bytes;
        gl::Error error = indexBuffer->reserveBufferSpace(bufferSizeRequired, type);
        if (error.isError())
        {
            return error;
        }

        void* output = NULL;
        error = indexBuffer->mapBuffer(bufferSizeRequired, &output, &streamOffset);
        if (error.isError())
        {
            return error;
        }

        convertIndices(type, destinationIndexType, staticBuffer ? storage->getData() : indices, convertCount, output);

        error = indexBuffer->unmapBuffer();
        if (error.isError())
        {
            return error;
        }

        if (staticBuffer)
        {
            streamOffset = (offset / typeInfo.bytes) * destTypeInfo.bytes;
            staticBuffer->getIndexRangeCache()->addRange(type, offset, count, translated->indexRange, streamOffset);
        }
    }

    translated->storage = directStorage ? storage : NULL;
    translated->indexBuffer = indexBuffer ? indexBuffer->getIndexBuffer() : NULL;
    translated->serial = directStorage ? storage->getSerial() : indexBuffer->getSerial();
    translated->startIndex = streamOffset / destTypeInfo.bytes;
    translated->startOffset = streamOffset;
    translated->indexType = destinationIndexType;

    if (storage)
    {
        storage->promoteStaticUsage(count * typeInfo.bytes);
    }

    return gl::Error(GL_NO_ERROR);
}

}
