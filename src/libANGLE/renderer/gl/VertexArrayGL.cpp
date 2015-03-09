//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// VertexArrayGL.cpp: Implements the class methods for VertexArrayGL.

#include "libANGLE/renderer/gl/VertexArrayGL.h"

#include "common/debug.h"
#include "libANGLE/Buffer.h"
#include "libANGLE/angletypes.h"
#include "libANGLE/renderer/gl/BufferGL.h"
#include "libANGLE/renderer/gl/FunctionsGL.h"
#include "libANGLE/renderer/gl/StateManagerGL.h"

namespace rx
{

VertexArrayGL::VertexArrayGL(const FunctionsGL *functions, StateManagerGL *stateManager)
    : VertexArrayImpl(),
      mFunctions(functions),
      mStateManager(stateManager),
      mVertexArrayID(0),
      mElementArrayBuffer(),
      mAttributes(),
      mAppliedElementArrayBuffer(0),
      mAppliedAttributes(),
      mScratchBufferSize(0),
      mStreamingScratchBuffer(0)
{
    ASSERT(mFunctions);
    ASSERT(mStateManager);
    mFunctions->genVertexArrays(1, &mVertexArrayID);

    // Set the cached vertex attribute array size
    GLint maxVertexAttribs;
    mFunctions->getIntegerv(GL_MAX_VERTEX_ATTRIBS, &maxVertexAttribs);
    mAttributes.resize(maxVertexAttribs);
    mAppliedAttributes.resize(maxVertexAttribs);
}

VertexArrayGL::~VertexArrayGL()
{
    if (mVertexArrayID != 0)
    {
        mFunctions->deleteVertexArrays(1, &mVertexArrayID);
        mVertexArrayID = 0;
    }

    if (mStreamingScratchBuffer != 0)
    {
        mFunctions->deleteBuffers(1, &mStreamingScratchBuffer);
        mScratchBufferSize = 0;
        mStreamingScratchBuffer = 0;
    }

    mElementArrayBuffer.set(nullptr);
    for (size_t idx = 0; idx < mAppliedAttributes.size(); idx++)
    {
        mAppliedAttributes[idx].buffer.set(NULL);
    }
}

void VertexArrayGL::setElementArrayBuffer(const gl::Buffer *buffer)
{
    mElementArrayBuffer.set(buffer);
}

void VertexArrayGL::setAttribute(size_t idx, const gl::VertexAttribute &attr)
{
    mAttributes[idx] = attr;
}

void VertexArrayGL::setAttributeDivisor(size_t idx, GLuint divisor)
{
    mAttributes[idx].divisor = divisor;
}

void VertexArrayGL::enableAttribute(size_t idx, bool enabledState)
{
    mAttributes[idx].enabled = enabledState;
}

void VertexArrayGL::setDrawState(GLint first, GLsizei count) const
{
    mStateManager->bindVertexArray(mVertexArrayID);

    GLuint elementArrayBufferID = 0;
    if (mElementArrayBuffer.get() != nullptr)
    {
        const BufferGL *bufferGL = GetImplAs<BufferGL>(mElementArrayBuffer.get());
        elementArrayBufferID = bufferGL->getBufferID();
    }

    if (elementArrayBufferID != mAppliedElementArrayBuffer)
    {
        mStateManager->bindVertexArray(mVertexArrayID);
        mStateManager->bindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementArrayBufferID);
        mStateManager->bindVertexArray(0);

        mAppliedElementArrayBuffer = elementArrayBufferID;
    }

    size_t streamingDataSize = 0;
    size_t maxAttributeDataSize = 0;
    for (size_t idx = 0; idx < mAttributes.size(); idx++)
    {
        // Always sync the enabled and divisor state, they are required for both streaming and buffered
        // attributes
        if (mAppliedAttributes[idx].enabled != mAttributes[idx].enabled)
        {
            if (mAttributes[idx].enabled)
            {
                mFunctions->enableVertexAttribArray(idx);
            }
            else
            {
                mFunctions->disableVertexAttribArray(idx);
            }
            mAppliedAttributes[idx].enabled = mAttributes[idx].enabled;
        }
        if (mAppliedAttributes[idx].divisor != mAttributes[idx].divisor)
        {
            mFunctions->vertexAttribDivisor(idx, mAttributes[idx].divisor);
            mAppliedAttributes[idx].divisor = mAttributes[idx].divisor;
        }

        if (mAttributes[idx].enabled && mAttributes[idx].buffer.get() == nullptr)
        {
            // If streaming is going to be required, compute the size of the required buffer
            // and how much slack space at the beginning of the buffer will be required by determining
            // the attribute with the largest data size.
            size_t typeSize = ComputeVertexAttributeTypeSize(mAttributes[idx]);
            streamingDataSize += typeSize * count;
            maxAttributeDataSize = std::max(maxAttributeDataSize, typeSize);
        }
        else
        {
            // Sync the attribute with no translation
            if (mAppliedAttributes[idx] != mAttributes[idx])
            {
                const gl::Buffer *arrayBuffer = mAttributes[idx].buffer.get();
                const BufferGL *arrayBufferGL = GetImplAs<BufferGL>(arrayBuffer);
                mStateManager->bindBuffer(GL_ARRAY_BUFFER, arrayBufferGL->getBufferID());

                if (mAttributes[idx].pureInteger)
                {
                    mFunctions->vertexAttribIPointer(idx, mAttributes[idx].size, mAttributes[idx].type,
                                                     mAttributes[idx].stride, mAttributes[idx].pointer);
                }
                else
                {
                    mFunctions->vertexAttribPointer(idx, mAttributes[idx].size, mAttributes[idx].type,
                                                    mAttributes[idx].normalized, mAttributes[idx].stride,
                                                    mAttributes[idx].pointer);
                }

                mAppliedAttributes[idx] = mAttributes[idx];
            }
        }
    }

    if (streamingDataSize > 0)
    {
        if (mStreamingScratchBuffer == 0)
        {
            mFunctions->genVertexArrays(1, &mStreamingScratchBuffer);
            mScratchBufferSize = 0;
        }

        // If first is greater than zero, a slack space needs to be left at the beginning of the buffer so that
        // the same 'first' argument can be passed into the draw call.
        const size_t bufferEmptySpace = maxAttributeDataSize * first;
        const size_t requiredBufferSize = streamingDataSize + bufferEmptySpace;

        mStateManager->bindBuffer(GL_ARRAY_BUFFER, mStreamingScratchBuffer);
        if (requiredBufferSize > mScratchBufferSize)
        {
            mFunctions->bufferData(GL_ARRAY_BUFFER, requiredBufferSize, nullptr, GL_DYNAMIC_DRAW);
            mScratchBufferSize = requiredBufferSize;
        }

        // Unmapping a buffer can return GL_FALSE to indicate that the system has corrupted the data
        // somehow (such as by a screen change), retry writing the data until it succeeds.
        GLboolean unmapResult = GL_FALSE;
        while (unmapResult != GL_TRUE)
        {
            uint8_t *bufferPointer = reinterpret_cast<uint8_t*>(mFunctions->mapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY));
            size_t curBufferOffset = bufferEmptySpace;

            for (size_t idx = 0; idx < mAttributes.size(); idx++)
            {
                if (mAttributes[idx].enabled && mAttributes[idx].buffer.get() == nullptr)
                {
                    const size_t sourceStride = ComputeVertexAttributeStride(mAttributes[idx]);
                    const size_t destStride = ComputeVertexAttributeTypeSize(mAttributes[idx]);

                    const uint8_t *inputPointer = reinterpret_cast<const uint8_t*>(mAttributes[idx].pointer) +
                                                  (first * sourceStride);

                    // Pack the data when copying it, user could have supplied a very large stride that would
                    // cause the buffer to be much larger than needed.
                    if (destStride == sourceStride)
                    {
                        // Can copy in one go, the data is packed
                        memcpy(bufferPointer + curBufferOffset, inputPointer, destStride * count);
                    }
                    else
                    {
                        // Copy each vertex individually
                        for (size_t vertexIdx = 0; vertexIdx < count; vertexIdx++)
                        {
                            memcpy(bufferPointer + curBufferOffset + (destStride * vertexIdx),
                                   inputPointer + (sourceStride * vertexIdx), destStride);
                        }
                    }

                    // Compute where the 0-index vertex would be.
                    const size_t vertexStartOffset = curBufferOffset - (first * destStride);

                    mFunctions->vertexAttribPointer(idx, mAttributes[idx].size, mAttributes[idx].type,
                                                    mAttributes[idx].normalized, destStride,
                                                    reinterpret_cast<const GLvoid*>(vertexStartOffset));

                    curBufferOffset += destStride * count;

                    // Mark the applied attribute as dirty by setting an invalid size so that if it doesn't
                    // need to be streamed later, there is no chance that the caching will skip it.
                    mAppliedAttributes[idx].size = -1;
                }
            }

            unmapResult = mFunctions->unmapBuffer(GL_ARRAY_BUFFER);
        }
    }
}

GLuint VertexArrayGL::getVertexArrayID() const
{
    return mVertexArrayID;
}

}
