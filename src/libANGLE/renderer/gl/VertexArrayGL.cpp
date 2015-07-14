//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// VertexArrayGL.cpp: Implements the class methods for VertexArrayGL.

#include "libANGLE/renderer/gl/VertexArrayGL.h"

#include "common/debug.h"
#include "common/mathutil.h"
#include "common/utilities.h"
#include "libANGLE/Buffer.h"
#include "libANGLE/angletypes.h"
#include "libANGLE/formatutils.h"
#include "libANGLE/renderer/gl/BufferGL.h"
#include "libANGLE/renderer/gl/FunctionsGL.h"
#include "libANGLE/renderer/gl/renderergl_utils.h"
#include "libANGLE/renderer/gl/StateManagerGL.h"

namespace rx
{

static size_t GetMaxAttributes(const FunctionsGL *functions)
{
    GLint maxVertexAttribs;
    functions->getIntegerv(GL_MAX_VERTEX_ATTRIBS, &maxVertexAttribs);
    return maxVertexAttribs;
}

VertexArrayStateGL::VertexArrayStateGL(size_t maxAttributes)
    : mStreamingElementArrayBuffer(0),
      mElementArrayBuffer(),
      mAttributes(maxAttributes)
{
}

const gl::VertexAttribute &VertexArrayStateGL::getAttribute(size_t idx) const
{
    ASSERT(idx < mAttributes.size());
    return mAttributes[idx];
}

gl::VertexAttribute &VertexArrayStateGL::getAttribute(size_t idx)
{
    ASSERT(idx < mAttributes.size());
    return mAttributes[idx];
}

void VertexArrayStateGL::setAttribute(size_t idx, const gl::VertexAttribute &attr)
{
    ASSERT(idx < mAttributes.size());
    mAttributes[idx] = attr;
}

void VertexArrayStateGL::setElementArrayBuffer(const gl::Buffer *buffer)
{
    mStreamingElementArrayBuffer = 0;
    mElementArrayBuffer.set(buffer);
}

void VertexArrayStateGL::setElementArrayBuffer(GLuint buffer)
{
    mStreamingElementArrayBuffer = buffer;
    mElementArrayBuffer.set(nullptr);
}

GLuint VertexArrayStateGL::getElementArrayBufferID() const
{
    if (mElementArrayBuffer.get() != nullptr)
    {
        ASSERT(mStreamingElementArrayBuffer == 0);
        return GetImplAs<BufferGL>(mElementArrayBuffer.get())->getBufferID();
    }
    else
    {
        return mStreamingElementArrayBuffer;
    }
}

const gl::Buffer *VertexArrayStateGL::getElementArrayBuffer() const
{
    return mElementArrayBuffer.get();
}

void VertexArrayStateGL::reset()
{
    mStreamingElementArrayBuffer = 0;
    mElementArrayBuffer.set(nullptr);
    for (auto &attribute : mAttributes)
    {
        attribute.buffer.set(nullptr);
    }
}

VertexArrayGL::VertexArrayGL(const gl::VertexArray::Data &data,
                             const FunctionsGL *functions,
                             StateManagerGL *stateManager)
    : VertexArrayImpl(data),
      mFunctions(functions),
      mStateManager(stateManager),
      mAbleToReadbackBufferData(false),
      mAbleToUseVAOs(false),
      mVertexArrayID(0),
      mLocalAppliedState(GetMaxAttributes(functions)),
      mStreamingElementArrayBufferSize(0),
      mStreamingElementArrayBuffer(0),
      mStreamingArrayBufferSize(0),
      mStreamingArrayBuffer(0)
{
    ASSERT(mFunctions);
    ASSERT(mStateManager);

    mAbleToReadbackBufferData = nativegl::AbleToReadbackBufferData(mFunctions);
    mAbleToUseVAOs = nativegl::AbleToUseVertexArrayObjects(mFunctions);
    if (mAbleToUseVAOs)
    {
        mFunctions->genVertexArrays(1, &mVertexArrayID);
    }
}

VertexArrayGL::~VertexArrayGL()
{
    if (mAbleToUseVAOs)
    {
        mStateManager->deleteVertexArray(mVertexArrayID);
        mVertexArrayID = 0;
    }

    mStateManager->deleteBuffer(mStreamingElementArrayBuffer);
    mStreamingElementArrayBufferSize = 0;
    mStreamingElementArrayBuffer = 0;

    mStateManager->deleteBuffer(mStreamingArrayBuffer);
    mStreamingArrayBufferSize = 0;
    mStreamingArrayBuffer = 0;

    mLocalAppliedState.reset();
}

gl::Error VertexArrayGL::syncDrawArraysState(const std::vector<GLuint> &activeAttribLocations, GLint first, GLsizei count) const
{
    return syncDrawState(activeAttribLocations, first, count, GL_NONE, nullptr, nullptr);
}

gl::Error VertexArrayGL::syncDrawElementsState(const std::vector<GLuint> &activeAttribLocations, GLsizei count,
                                               GLenum type, const GLvoid *indices, const GLvoid **outIndices) const
{
    return syncDrawState(activeAttribLocations, 0, count, type, indices, outIndices);
}

gl::Error VertexArrayGL::syncDrawState(const std::vector<GLuint> &activeAttribLocations, GLint first, GLsizei count, GLenum type, const GLvoid *indices, const GLvoid **outIndices) const
{
    // Check if any attributes need to be streamed, determines if the index range needs to be computed
    bool attributesNeedStreaming = doAttributesNeedStreaming(activeAttribLocations);

    // Check if readback is required.  This determines if we need to use the default VAO or the
    // generated one
    bool requiresBufferReadback = doesBufferDataNeedToBeRead(attributesNeedStreaming, type != GL_NONE);

    // Use of vertex array objects requires that no client side data is used for draw calls so the
    // platform must be able to stream vertex data.  This requires being able to read back buffers
    // sometimes when client side vertex data is used but a buffer is used for index data.
    bool useDefaultVAO = !mAbleToUseVAOs || (!mAbleToReadbackBufferData && requiresBufferReadback);

    // Can't use the default VAO on the core profile, this shouldn't happen.
    ASSERT(!(useDefaultVAO && (mFunctions->profile & GL_CONTEXT_CORE_PROFILE_BIT)));

    // Sync the default VAO's current state if required.
    VertexArrayStateGL *appliedState = useDefaultVAO ? mStateManager->getDefaultVertexArrayState() : &mLocalAppliedState;

    // Apply VAO state if needed
    if (useDefaultVAO)
    {
        if (mAbleToUseVAOs)
        {
            mStateManager->bindVertexArray(0, appliedState->getElementArrayBufferID());
        }

        // Don't need to stream attributes when using the default VAO, client side data is acceptable.
        attributesNeedStreaming = false;
    }
    else
    {
        mStateManager->bindVertexArray(mVertexArrayID, appliedState->getElementArrayBufferID());
    }

    // Determine if an index buffer needs to be streamed and the range of vertices that need to be copied
    gl::RangeUI indexRange(0, 0);
    if (type != GL_NONE)
    {
        gl::Error error = syncIndexData(appliedState, count, type, indices, attributesNeedStreaming, &indexRange, outIndices);
        if (error.isError())
        {
            return error;
        }
    }
    else
    {
        // Not an indexed call, set the range to [first, first + count)
        indexRange.start = first;
        indexRange.end = first + count;
    }

    // Sync the vertex attribute state and track what data needs to be streamed
    size_t streamingDataSize = 0;
    size_t maxAttributeDataSize = 0;
    gl::Error error = syncAttributeState(appliedState, activeAttribLocations, attributesNeedStreaming, indexRange,
                                         &streamingDataSize, &maxAttributeDataSize);
    if (error.isError())
    {
        return error;
    }

    if (streamingDataSize > 0)
    {
        ASSERT(attributesNeedStreaming);

        error = streamAttributes(appliedState, activeAttribLocations, streamingDataSize, maxAttributeDataSize,
                                 indexRange);
        if (error.isError())
        {
            return error;
        }
    }

    return gl::Error(GL_NO_ERROR);
}

bool VertexArrayGL::doAttributesNeedStreaming(const std::vector<GLuint> &activeAttribLocations) const
{
    const auto &attribs = mData.getVertexAttributes();
    for (size_t activeAttrib = 0; activeAttrib < activeAttribLocations.size(); activeAttrib++)
    {
        GLuint idx = activeAttribLocations[activeAttrib];
        if (attribs[idx].enabled && attribs[idx].buffer.get() == nullptr)
        {
            return true;
        }
    }

    return false;
}

bool VertexArrayGL::doesBufferDataNeedToBeRead(bool attributesNeedStreaming, bool indexedDrawCall) const
{
    // Need to read back vertex buffer data when doing an draw call that requires vertex streaming and uses an index buffer.
    return attributesNeedStreaming && indexedDrawCall && mData.getElementArrayBuffer().get() != nullptr;
}

gl::Error VertexArrayGL::syncAttributeState(VertexArrayStateGL *appliedState, const std::vector<GLuint> &activeAttribLocations, bool attributesNeedStreaming,
                                            const gl::RangeUI &indexRange,  size_t *outStreamingDataSize, size_t *outMaxAttributeDataSize) const
{
    *outStreamingDataSize = 0;
    *outMaxAttributeDataSize = 0;

    const auto &attribs = mData.getVertexAttributes();
    for (size_t activeAttrib = 0; activeAttrib < activeAttribLocations.size(); activeAttrib++)
    {
        GLuint idx = activeAttribLocations[activeAttrib];
        const auto &attrib = attribs[idx];
        auto &appliedAttrib = appliedState->getAttribute(idx);

        // Always sync the enabled and divisor state, they are required for both streaming and buffered
        // attributes
        if (appliedAttrib.enabled != attrib.enabled)
        {
            if (attrib.enabled)
            {
                mFunctions->enableVertexAttribArray(idx);
            }
            else
            {
                mFunctions->disableVertexAttribArray(idx);
            }
            appliedAttrib.enabled = attrib.enabled;
        }
        if (appliedAttrib.divisor != attrib.divisor)
        {
            mFunctions->vertexAttribDivisor(idx, attrib.divisor);
            appliedAttrib.divisor = attrib.divisor;
        }

        if (attribs[idx].enabled && attrib.buffer.get() == nullptr && attributesNeedStreaming)
        {
            const size_t streamedVertexCount = indexRange.end - indexRange.start + 1;

            // If streaming is going to be required, compute the size of the required buffer
            // and how much slack space at the beginning of the buffer will be required by determining
            // the attribute with the largest data size.
            size_t typeSize = ComputeVertexAttributeTypeSize(attrib);
            *outStreamingDataSize += typeSize * streamedVertexCount;
            *outMaxAttributeDataSize = std::max(*outMaxAttributeDataSize, typeSize);
        }
        else
        {
            // Sync the attribute with no translation
            if (appliedAttrib != attrib)
            {
                const gl::Buffer *arrayBuffer = attrib.buffer.get();
                if (arrayBuffer != nullptr)
                {
                    const BufferGL *arrayBufferGL = GetImplAs<BufferGL>(arrayBuffer);
                    mStateManager->bindBuffer(GL_ARRAY_BUFFER, arrayBufferGL->getBufferID());
                }
                else
                {
                    mStateManager->bindBuffer(GL_ARRAY_BUFFER, 0);
                }

                if (attrib.pureInteger)
                {
                    mFunctions->vertexAttribIPointer(idx, attrib.size, attrib.type,
                                                     attrib.stride, attrib.pointer);
                }
                else
                {
                    mFunctions->vertexAttribPointer(idx, attrib.size, attrib.type,
                                                    attrib.normalized, attrib.stride,
                                                    attrib.pointer);
                }

                appliedAttrib = attrib;
            }
        }
    }

    return gl::Error(GL_NO_ERROR);
}

gl::Error VertexArrayGL::syncIndexData(VertexArrayStateGL *appliedState, GLsizei count, GLenum type, const GLvoid *indices, bool attributesNeedStreaming,
                                       gl::RangeUI *outIndexRange, const GLvoid **outIndices) const
{
    ASSERT(outIndices);

    gl::Buffer *elementArrayBuffer = mData.getElementArrayBuffer().get();

    // Need to check the range of indices if attributes need to be streamed
    if (elementArrayBuffer != nullptr)
    {
        if (elementArrayBuffer != appliedState->getElementArrayBuffer())
        {
            const BufferGL *bufferGL = GetImplAs<BufferGL>(elementArrayBuffer);
            mStateManager->bindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufferGL->getBufferID());
            appliedState->setElementArrayBuffer(elementArrayBuffer);
        }

        // Only compute the index range if the attributes also need to be streamed
        if (attributesNeedStreaming)
        {
            ptrdiff_t elementArrayBufferOffset = reinterpret_cast<ptrdiff_t>(indices);
            gl::Error error = mData.getElementArrayBuffer()->getIndexRange(type, static_cast<size_t>(elementArrayBufferOffset), count, outIndexRange);
            if (error.isError())
            {
                return error;
            }
        }

        // Indices serves as an offset into the index buffer in this case, use the same value for the draw call
        *outIndices = indices;
    }
    else
    {
        if (attributesNeedStreaming)
        {
            // Need to stream the index buffer

            // Only compute the index range if the attributes also need to be streamed
            if (attributesNeedStreaming)
            {
                *outIndexRange = gl::ComputeIndexRange(type, indices, count);
            }

            // Allocate the streaming element array buffer
            if (mStreamingElementArrayBuffer == 0)
            {
                mFunctions->genBuffers(1, &mStreamingElementArrayBuffer);
                mStreamingElementArrayBufferSize = 0;
            }

            mStateManager->bindBuffer(GL_ELEMENT_ARRAY_BUFFER, mStreamingElementArrayBuffer);
            appliedState->setElementArrayBuffer(mStreamingElementArrayBuffer);

            // Make sure the element array buffer is large enough
            const gl::Type &indexTypeInfo = gl::GetTypeInfo(type);
            size_t requiredStreamingBufferSize = indexTypeInfo.bytes * count;
            if (requiredStreamingBufferSize > mStreamingElementArrayBufferSize)
            {
                // Copy the indices in while resizing the buffer
                mFunctions->bufferData(GL_ELEMENT_ARRAY_BUFFER, requiredStreamingBufferSize, indices, GL_DYNAMIC_DRAW);
                mStreamingElementArrayBufferSize = requiredStreamingBufferSize;
            }
            else
            {
                // Put the indices at the beginning of the buffer
                mFunctions->bufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, requiredStreamingBufferSize, indices);
            }

            // Set the index offset for the draw call to zero since the supplied index pointer is to client data
            *outIndices = nullptr;
        }
        else
        {
            // Can directly apply the indicies pointer
            mStateManager->bindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
            appliedState->setElementArrayBuffer(0u);

            *outIndices = indices;
        }
    }

    return gl::Error(GL_NO_ERROR);
}

gl::Error VertexArrayGL::streamAttributes(VertexArrayStateGL *appliedState, const std::vector<GLuint> &activeAttribLocations, size_t streamingDataSize,
                                          size_t maxAttributeDataSize, const gl::RangeUI &indexRange) const
{
    if (mStreamingArrayBuffer == 0)
    {
        mFunctions->genBuffers(1, &mStreamingArrayBuffer);
        mStreamingArrayBufferSize = 0;
    }

    // If first is greater than zero, a slack space needs to be left at the beginning of the buffer so that
    // the same 'first' argument can be passed into the draw call.
    const size_t bufferEmptySpace = maxAttributeDataSize * indexRange.start;
    const size_t requiredBufferSize = streamingDataSize + bufferEmptySpace;

    mStateManager->bindBuffer(GL_ARRAY_BUFFER, mStreamingArrayBuffer);
    if (requiredBufferSize > mStreamingArrayBufferSize)
    {
        mFunctions->bufferData(GL_ARRAY_BUFFER, requiredBufferSize, nullptr, GL_DYNAMIC_DRAW);
        mStreamingArrayBufferSize = requiredBufferSize;
    }

    // Unmapping a buffer can return GL_FALSE to indicate that the system has corrupted the data
    // somehow (such as by a screen change), retry writing the data a few times and return OUT_OF_MEMORY
    // if that fails.
    bool unmapResult = false;
    size_t unmapRetryAttempts = 5;
    while (!unmapResult && --unmapRetryAttempts > 0)
    {
        uint8_t *bufferPointer = nativegl::MapBuffer(mFunctions, GL_ARRAY_BUFFER, 0, requiredBufferSize, GL_MAP_WRITE_BIT);
        ASSERT(bufferPointer != nullptr);

        size_t curBufferOffset = bufferEmptySpace;

        const size_t streamedVertexCount = indexRange.end - indexRange.start + 1;

        const auto &attribs = mData.getVertexAttributes();
        for (size_t activeAttrib = 0; activeAttrib < activeAttribLocations.size(); activeAttrib++)
        {
            GLuint idx = activeAttribLocations[activeAttrib];
            const auto &attrib = attribs[idx];
            auto &appliedAttrib = appliedState->getAttribute(idx);

            if (attrib.enabled && attrib.buffer.get() == nullptr)
            {
                const size_t sourceStride = ComputeVertexAttributeStride(attrib);
                const size_t destStride = ComputeVertexAttributeTypeSize(attrib);

                const uint8_t *inputPointer = reinterpret_cast<const uint8_t*>(attrib.pointer);

                // Pack the data when copying it, user could have supplied a very large stride that would
                // cause the buffer to be much larger than needed.
                if (destStride == sourceStride)
                {
                    // Can copy in one go, the data is packed
                    memcpy(bufferPointer + curBufferOffset,
                           inputPointer + (sourceStride * indexRange.start),
                           destStride * streamedVertexCount);
                }
                else
                {
                    // Copy each vertex individually
                    for (size_t vertexIdx = indexRange.start; vertexIdx <= indexRange.end; vertexIdx++)
                    {
                        memcpy(bufferPointer + curBufferOffset + (destStride * vertexIdx),
                               inputPointer + (sourceStride * vertexIdx),
                               destStride);
                    }
                }

                // Compute where the 0-index vertex would be.
                const size_t vertexStartOffset = curBufferOffset - (indexRange.start * destStride);

                mFunctions->vertexAttribPointer(
                    idx, attrib.size, attrib.type, attrib.normalized,
                    static_cast<GLsizei>(destStride),
                    reinterpret_cast<const GLvoid *>(vertexStartOffset));

                curBufferOffset += destStride * streamedVertexCount;

                // Mark the applied attribute as dirty by setting an invalid size so that if it doesn't
                // need to be streamed later, there is no chance that the caching will skip it.
                appliedAttrib.size = static_cast<GLuint>(-1);
            }
        }

        unmapResult = nativegl::UnmapBuffer(mFunctions, GL_ARRAY_BUFFER);
    }

    if (!unmapResult)
    {
        return gl::Error(GL_OUT_OF_MEMORY, "Failed to unmap the client data streaming buffer.");
    }

    return gl::Error(GL_NO_ERROR);
}

}
