//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// VertexArrayGL.cpp: Implements the class methods for VertexArrayGL.

#include "libANGLE/renderer/gl/VertexArrayGL.h"

#include "common/bitset_utils.h"
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

using namespace gl;

namespace rx
{

VertexArrayGL::VertexArrayGL(const VertexArrayState &state,
                             const FunctionsGL *functions,
                             StateManagerGL *stateManager)
    : VertexArrayImpl(state),
      mFunctions(functions),
      mStateManager(stateManager),
      mVertexArrayID(0),
      mAppliedElementArrayBuffer(),
      mAppliedBindings(state.getMaxBindings()),
      mStreamingElementArrayBufferSize(0),
      mStreamingElementArrayBuffer(0),
      mStreamingArrayBufferSize(0),
      mStreamingArrayBuffer(0)
{
    ASSERT(mFunctions);
    ASSERT(mStateManager);
    mFunctions->genVertexArrays(1, &mVertexArrayID);

    // Set the cached vertex attribute array and vertex attribute binding array size
    GLuint maxVertexAttribs = static_cast<GLuint>(state.getMaxAttribs());
    for (GLuint i = 0; i < maxVertexAttribs; i++)
    {
        mAppliedAttributes.emplace_back(i);
    }
}

VertexArrayGL::~VertexArrayGL()
{
    mStateManager->deleteVertexArray(mVertexArrayID);
    mVertexArrayID = 0;

    mStateManager->deleteBuffer(mStreamingElementArrayBuffer);
    mStreamingElementArrayBufferSize = 0;
    mStreamingElementArrayBuffer     = 0;

    mStateManager->deleteBuffer(mStreamingArrayBuffer);
    mStreamingArrayBufferSize = 0;
    mStreamingArrayBuffer     = 0;

    mAppliedElementArrayBuffer.set(nullptr);
    for (auto &binding : mAppliedBindings)
    {
        binding.buffer.set(nullptr);
    }
}

bool VertexArrayGL::attributeNeedsStreaming(size_t attribIndex) const
{
    const auto &attrib  = mData.getVertexAttribute(attribIndex);
    const auto &binding = mData.getBindingFromAttribIndex(attribIndex);
    return (attrib.enabled && binding.buffer.get() == nullptr);
}

gl::Error VertexArrayGL::syncDrawArraysState(const gl::AttributesMask &activeAttributesMask,
                                             GLint first,
                                             GLsizei count,
                                             GLsizei instanceCount) const
{
    return syncDrawState(activeAttributesMask, first, count, GL_NONE, nullptr, instanceCount, false,
                         nullptr);
}

gl::Error VertexArrayGL::syncDrawElementsState(const gl::AttributesMask &activeAttributesMask,
                                               GLsizei count,
                                               GLenum type,
                                               const GLvoid *indices,
                                               GLsizei instanceCount,
                                               bool primitiveRestartEnabled,
                                               const GLvoid **outIndices) const
{
    return syncDrawState(activeAttributesMask, 0, count, type, indices, instanceCount,
                         primitiveRestartEnabled, outIndices);
}

gl::Error VertexArrayGL::syncElementArrayState() const
{
    gl::Buffer *elementArrayBuffer = mData.getElementArrayBuffer().get();
    ASSERT(elementArrayBuffer);
    if (elementArrayBuffer != mAppliedElementArrayBuffer.get())
    {
        const BufferGL *bufferGL = GetImplAs<BufferGL>(elementArrayBuffer);
        mStateManager->bindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufferGL->getBufferID());
        mAppliedElementArrayBuffer.set(elementArrayBuffer);
    }

    return gl::NoError();
}

gl::Error VertexArrayGL::syncDrawState(const gl::AttributesMask &activeAttributesMask,
                                       GLint first,
                                       GLsizei count,
                                       GLenum type,
                                       const GLvoid *indices,
                                       GLsizei instanceCount,
                                       bool primitiveRestartEnabled,
                                       const GLvoid **outIndices) const
{
    mStateManager->bindVertexArray(mVertexArrayID, getAppliedElementArrayBufferID());

    // Check if any attributes need to be streamed, determines if the index range needs to be
    // computed
    bool attributesNeedStreaming = mAttributesNeedStreaming.any();

    // Determine if an index buffer needs to be streamed and the range of vertices that need to be
    // copied
    IndexRange indexRange;
    if (type != GL_NONE)
    {
        Error error = syncIndexData(count, type, indices, primitiveRestartEnabled,
                                    attributesNeedStreaming, &indexRange, outIndices);
        if (error.isError())
        {
            return error;
        }
    }
    else
    {
        // Not an indexed call, set the range to [first, first + count - 1]
        indexRange.start = first;
        indexRange.end   = first + count - 1;
    }

    if (attributesNeedStreaming)
    {
        Error error = streamAttributes(activeAttributesMask, instanceCount, indexRange);
        if (error.isError())
        {
            return error;
        }
    }

    return NoError();
}

gl::Error VertexArrayGL::syncIndexData(GLsizei count,
                                       GLenum type,
                                       const GLvoid *indices,
                                       bool primitiveRestartEnabled,
                                       bool attributesNeedStreaming,
                                       IndexRange *outIndexRange,
                                       const GLvoid **outIndices) const
{
    ASSERT(outIndices);

    gl::Buffer *elementArrayBuffer = mData.getElementArrayBuffer().get();

    // Need to check the range of indices if attributes need to be streamed
    if (elementArrayBuffer != nullptr)
    {
        if (elementArrayBuffer != mAppliedElementArrayBuffer.get())
        {
            const BufferGL *bufferGL = GetImplAs<BufferGL>(elementArrayBuffer);
            mStateManager->bindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufferGL->getBufferID());
            mAppliedElementArrayBuffer.set(elementArrayBuffer);
        }

        // Only compute the index range if the attributes also need to be streamed
        if (attributesNeedStreaming)
        {
            ptrdiff_t elementArrayBufferOffset = reinterpret_cast<ptrdiff_t>(indices);
            Error error                        = mData.getElementArrayBuffer()->getIndexRange(
                type, elementArrayBufferOffset, count, primitiveRestartEnabled, outIndexRange);
            if (error.isError())
            {
                return error;
            }
        }

        // Indices serves as an offset into the index buffer in this case, use the same value for
        // the draw call
        *outIndices = indices;
    }
    else
    {
        // Need to stream the index buffer
        // TODO: if GLES, nothing needs to be streamed

        // Only compute the index range if the attributes also need to be streamed
        if (attributesNeedStreaming)
        {
            *outIndexRange = ComputeIndexRange(type, indices, count, primitiveRestartEnabled);
        }

        // Allocate the streaming element array buffer
        if (mStreamingElementArrayBuffer == 0)
        {
            mFunctions->genBuffers(1, &mStreamingElementArrayBuffer);
            mStreamingElementArrayBufferSize = 0;
        }

        mStateManager->bindBuffer(GL_ELEMENT_ARRAY_BUFFER, mStreamingElementArrayBuffer);
        mAppliedElementArrayBuffer.set(nullptr);

        // Make sure the element array buffer is large enough
        const Type &indexTypeInfo          = GetTypeInfo(type);
        size_t requiredStreamingBufferSize = indexTypeInfo.bytes * count;
        if (requiredStreamingBufferSize > mStreamingElementArrayBufferSize)
        {
            // Copy the indices in while resizing the buffer
            mFunctions->bufferData(GL_ELEMENT_ARRAY_BUFFER, requiredStreamingBufferSize, indices,
                                   GL_DYNAMIC_DRAW);
            mStreamingElementArrayBufferSize = requiredStreamingBufferSize;
        }
        else
        {
            // Put the indices at the beginning of the buffer
            mFunctions->bufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, requiredStreamingBufferSize,
                                      indices);
        }

        // Set the index offset for the draw call to zero since the supplied index pointer is to
        // client data
        *outIndices = nullptr;
    }

    return NoError();
}

void VertexArrayGL::computeStreamingAttributeSizes(const gl::AttributesMask &activeAttributesMask,
                                                   GLsizei instanceCount,
                                                   const gl::IndexRange &indexRange,
                                                   size_t *outStreamingDataSize,
                                                   size_t *outMaxAttributeDataSize) const
{
    *outStreamingDataSize    = 0;
    *outMaxAttributeDataSize = 0;

    ASSERT(mAttributesNeedStreaming.any());

    const auto &attribs  = mData.getVertexAttributes();
    const auto &bindings = mData.getVertexBindings();

    gl::AttributesMask attribsToStream = (mAttributesNeedStreaming & activeAttributesMask);

    for (auto idx : attribsToStream)
    {
        ASSERT(attributeNeedsStreaming(idx));

        const auto &attrib  = attribs[idx];
        const auto &binding = bindings[attrib.bindingIndex];

        // If streaming is going to be required, compute the size of the required buffer
        // and how much slack space at the beginning of the buffer will be required by determining
        // the attribute with the largest data size.
        size_t typeSize = ComputeVertexAttributeTypeSize(attrib);
        *outStreamingDataSize += typeSize * ComputeVertexBindingElementCount(
                                                binding, indexRange.vertexCount(), instanceCount);
        *outMaxAttributeDataSize = std::max(*outMaxAttributeDataSize, typeSize);
    }
}

gl::Error VertexArrayGL::streamAttributes(const gl::AttributesMask &activeAttributesMask,
                                          GLsizei instanceCount,
                                          const gl::IndexRange &indexRange) const
{
    // Sync the vertex attribute state and track what data needs to be streamed
    size_t streamingDataSize    = 0;
    size_t maxAttributeDataSize = 0;

    computeStreamingAttributeSizes(activeAttributesMask, instanceCount, indexRange,
                                   &streamingDataSize, &maxAttributeDataSize);

    if (streamingDataSize == 0)
    {
        return gl::NoError();
    }

    if (mStreamingArrayBuffer == 0)
    {
        mFunctions->genBuffers(1, &mStreamingArrayBuffer);
        mStreamingArrayBufferSize = 0;
    }

    // If first is greater than zero, a slack space needs to be left at the beginning of the buffer
    // so that the same 'first' argument can be passed into the draw call.
    const size_t bufferEmptySpace   = maxAttributeDataSize * indexRange.start;
    const size_t requiredBufferSize = streamingDataSize + bufferEmptySpace;

    mStateManager->bindBuffer(GL_ARRAY_BUFFER, mStreamingArrayBuffer);
    if (requiredBufferSize > mStreamingArrayBufferSize)
    {
        mFunctions->bufferData(GL_ARRAY_BUFFER, requiredBufferSize, nullptr, GL_DYNAMIC_DRAW);
        mStreamingArrayBufferSize = requiredBufferSize;
    }

    // Unmapping a buffer can return GL_FALSE to indicate that the system has corrupted the data
    // somehow (such as by a screen change), retry writing the data a few times and return
    // OUT_OF_MEMORY if that fails.
    GLboolean unmapResult     = GL_FALSE;
    size_t unmapRetryAttempts = 5;
    while (unmapResult != GL_TRUE && --unmapRetryAttempts > 0)
    {
        uint8_t *bufferPointer = MapBufferRangeWithFallback(mFunctions, GL_ARRAY_BUFFER, 0,
                                                            requiredBufferSize, GL_MAP_WRITE_BIT);
        size_t curBufferOffset = bufferEmptySpace;

        const auto &attribs  = mData.getVertexAttributes();
        const auto &bindings = mData.getVertexBindings();

        gl::AttributesMask attribsToStream = (mAttributesNeedStreaming & activeAttributesMask);

        for (auto idx : attribsToStream)
        {
            ASSERT(attributeNeedsStreaming(idx));

            const auto &attrib  = attribs[idx];
            const auto &binding = bindings[attrib.bindingIndex];

            const size_t streamedVertexCount =
                ComputeVertexBindingElementCount(binding, indexRange.vertexCount(), instanceCount);

            const size_t sourceStride = ComputeVertexAttributeStride(attrib, binding);
            const size_t destStride   = ComputeVertexAttributeTypeSize(attrib);

            // Vertices do not apply the 'start' offset when the divisor is non-zero even when doing
            // a non-instanced draw call
            const size_t firstIndex = binding.divisor == 0 ? indexRange.start : 0;

            // Attributes using client memory ignore the VERTEX_ATTRIB_BINDING state.
            // https://www.opengl.org/registry/specs/ARB/vertex_attrib_binding.txt
            const uint8_t *inputPointer = reinterpret_cast<const uint8_t *>(attrib.pointer);

            // Pack the data when copying it, user could have supplied a very large stride that
            // would cause the buffer to be much larger than needed.
            if (destStride == sourceStride)
            {
                // Can copy in one go, the data is packed
                memcpy(bufferPointer + curBufferOffset, inputPointer + (sourceStride * firstIndex),
                       destStride * streamedVertexCount);
            }
            else
            {
                // Copy each vertex individually
                for (size_t vertexIdx = 0; vertexIdx < streamedVertexCount; vertexIdx++)
                {
                    uint8_t *out      = bufferPointer + curBufferOffset + (destStride * vertexIdx);
                    const uint8_t *in = inputPointer + sourceStride * (vertexIdx + firstIndex);
                    memcpy(out, in, destStride);
                }
            }

            // Compute where the 0-index vertex would be.
            const size_t vertexStartOffset = curBufferOffset - (firstIndex * destStride);

            callVertexAttribPointer(static_cast<GLuint>(idx), attrib.size, attrib.type,
                                    attrib.normalized, static_cast<GLsizei>(destStride),
                                    static_cast<GLintptr>(vertexStartOffset), attrib.pureInteger);

            curBufferOffset += destStride * streamedVertexCount;
        }

        unmapResult = mFunctions->unmapBuffer(GL_ARRAY_BUFFER);
    }

    if (unmapResult != GL_TRUE)
    {
        return Error(GL_OUT_OF_MEMORY, "Failed to unmap the client data streaming buffer.");
    }

    return NoError();
}

GLuint VertexArrayGL::getVertexArrayID() const
{
    return mVertexArrayID;
}

GLuint VertexArrayGL::getAppliedElementArrayBufferID() const
{
    if (mAppliedElementArrayBuffer.get() == nullptr)
    {
        return mStreamingElementArrayBuffer;
    }

    return GetImplAs<BufferGL>(mAppliedElementArrayBuffer.get())->getBufferID();
}

void VertexArrayGL::updateNeedsStreaming(size_t attribIndex)
{
    mAttributesNeedStreaming.set(attribIndex, attributeNeedsStreaming(attribIndex));
}

bool VertexArrayGL::canUseVertexAttribPointer(size_t attribIndex) const
{
    const auto &attrib = mData.getVertexAttribute(attribIndex);
    return attribIndex == attrib.bindingIndex && attrib.relativeOffset == 0;
}

bool VertexArrayGL::arrayBufferUsageChanged(size_t bindingIndex) const
{
    const auto *appliedBuffer = mAppliedBindings[bindingIndex].buffer.get();
    const auto *newBuffer     = mData.getVertexBinding(bindingIndex).buffer.get();
    return (appliedBuffer != newBuffer && (appliedBuffer == nullptr || newBuffer == nullptr));
}

bool VertexArrayGL::vertexAttribEnabledChanged(size_t attribIndex) const
{
    const auto &attrib = mData.getVertexAttribute(attribIndex);
    return attrib.enabled != mAppliedAttributes[attribIndex].enabled;
}

bool VertexArrayGL::vertexAttribPointerChanged(size_t attribIndex) const
{
    ASSERT(canUseVertexAttribPointer(attribIndex));

    const auto &attrib  = mData.getVertexAttribute(attribIndex);
    const auto &binding = mData.getVertexBinding(attrib.bindingIndex);
    return !sameVertexFormat(attrib, mAppliedAttributes[attribIndex]) ||
           !sameBindingBuffer(binding, mAppliedBindings[attribIndex]);
}

bool VertexArrayGL::vertexBindingDivisorChanged(size_t bindingIndex) const
{
    const auto &binding = mData.getVertexBinding(bindingIndex);
    return binding.divisor != mAppliedBindings[bindingIndex].divisor;
}

void VertexArrayGL::callVertexAttribPointer(GLuint attribIndex,
                                            GLuint size,
                                            GLenum type,
                                            GLboolean normalized,
                                            GLsizei stride,
                                            GLintptr offset,
                                            GLboolean pureInteger) const
{
    const GLvoid *pointer = reinterpret_cast<const GLvoid *>(offset);
    if (pureInteger)
    {
        ASSERT(!normalized);
        mFunctions->vertexAttribIPointer(attribIndex, size, type, stride, pointer);
    }
    else
    {
        mFunctions->vertexAttribPointer(attribIndex, size, type, normalized, stride, pointer);
    }
}

void VertexArrayGL::updateAttribEnabled(size_t attribIndex)
{
    ASSERT(mAttributesNeedStreaming.test(attribIndex) == attributeNeedsStreaming(attribIndex));

    const VertexAttribute &attrib = mData.getVertexAttribute(attribIndex);
    if (attrib.enabled)
    {
        mFunctions->enableVertexAttribArray(static_cast<GLuint>(attribIndex));
    }
    else
    {
        mFunctions->disableVertexAttribArray(static_cast<GLuint>(attribIndex));
    }

    mAppliedAttributes[attribIndex].enabled = attrib.enabled;
}

bool VertexArrayGL::usesClientMemoryPointer(size_t attribIndex) const
{
    return mData.getVertexBinding(attribIndex).buffer.get() == nullptr;
}

void VertexArrayGL::updateClientMemoryPointer(size_t attribIndex)
{
    ASSERT(canUseVertexAttribPointer(attribIndex));
    ASSERT(mAttributesNeedStreaming.test(attribIndex) == attributeNeedsStreaming(attribIndex));
    ASSERT(mData.getVertexBinding(attribIndex).buffer.get() == nullptr);

    // If we need to stream, defer the attribPointer to the draw call;
    mAppliedBindings[attribIndex].buffer.set(nullptr);
}

void VertexArrayGL::updateAttribPointerWithArrayBuffer(size_t attribIndex)
{
    ASSERT(canUseVertexAttribPointer(attribIndex));
    // Attributes using client memory pointer should be handled in updateClientMemoryPointer().
    ASSERT(!usesClientMemoryPointer(attribIndex));

    const auto &binding       = mData.getVertexBinding(attribIndex);
    const Buffer *arrayBuffer = binding.buffer.get();
    ASSERT(arrayBuffer != nullptr);
    const BufferGL *arrayBufferGL = GetImplAs<BufferGL>(arrayBuffer);
    mStateManager->bindBuffer(GL_ARRAY_BUFFER, arrayBufferGL->getBufferID());

    const auto &attrib = mData.getVertexAttribute(attribIndex);
    callVertexAttribPointer(static_cast<GLuint>(attribIndex), attrib.size, attrib.type,
                            attrib.normalized, binding.stride, binding.offset, attrib.pureInteger);

    mAppliedAttributes[attribIndex].size           = attrib.size;
    mAppliedAttributes[attribIndex].type           = attrib.type;
    mAppliedAttributes[attribIndex].normalized     = attrib.normalized;
    mAppliedAttributes[attribIndex].pureInteger    = attrib.pureInteger;
    mAppliedAttributes[attribIndex].relativeOffset = attrib.relativeOffset;

    mAppliedAttributes[attribIndex].bindingIndex = attrib.bindingIndex;

    mAppliedBindings[attribIndex].stride = binding.stride;
    mAppliedBindings[attribIndex].offset = binding.offset;
    mAppliedBindings[attribIndex].buffer = binding.buffer;
}

void VertexArrayGL::updateAttribDivisor(size_t attribIndex)
{
    ASSERT(attribIndex == mData.getBindingIndexFromAttribIndex(attribIndex));

    const VertexBinding &binding = mData.getVertexBinding(attribIndex);
    mFunctions->vertexAttribDivisor(static_cast<GLuint>(attribIndex), binding.divisor);

    mAppliedBindings[attribIndex].divisor = binding.divisor;
}

void VertexArrayGL::syncState(ContextImpl *contextImpl, const VertexArray::DirtyBits &dirtyBits)
{
    // TODO(jmadill): Element array buffer bindings

    mStateManager->bindVertexArray(mVertexArrayID, getAppliedElementArrayBufferID());

    // TODO(jiawei.shao@intel.com): Vertex Attrib Bindings
    for (size_t index = 0; index < mData.getMaxAttribs(); index++)
    {
        bool dirtyAttribEnabled = dirtyBits[VertexArray::DIRTY_BIT_ATTRIB_0_ENABLED + index] &&
                                  vertexAttribEnabledChanged(index);
        bool dirtyArrayBufferUsage = dirtyBits[VertexArray::DIRTY_BIT_ATTRIB_0_POINTER + index] &&
                                     arrayBufferUsageChanged(index);
        bool dirtyAttribDivisor = dirtyBits[VertexArray::DIRTY_BIT_BINDING_0_DIVISOR + index] &&
                                  vertexBindingDivisorChanged(index);

        // Update if we need to stream when we enable/disable an attribute or change the usage of
        // the vertex buffer (using buffer or client memory pointer).
        if (dirtyAttribEnabled || dirtyArrayBufferUsage)
        {
            updateNeedsStreaming(index);
        }

        if (dirtyAttribEnabled)
        {
            updateAttribEnabled(index);
        }

        if (dirtyBits[VertexArray::DIRTY_BIT_ATTRIB_0_POINTER + index])
        {
            // Since ANGLE always uses a non-zero VAO, we cannot use a client memory pointer on it:
            // [OpenGL ES 3.0.2] Section 2.8 page 24:
            // An INVALID_OPERATION error is generated when a non-zero vertex array object is bound,
            // zero is bound to the ARRAY_BUFFER buffer object binding point, and the pointer
            // argument is not NULL.
            if (usesClientMemoryPointer(index))
            {
                // Update the cache when we starts using a client memory pointer instead of a
                // buffer.
                if (dirtyArrayBufferUsage)
                {
                    updateClientMemoryPointer(index);
                }
            }
            // Call VertexAttribPointer with new parameters or a different array buffer.
            else if (dirtyArrayBufferUsage || vertexAttribPointerChanged(index))
            {
                updateAttribPointerWithArrayBuffer(index);
            }
        }

        if (dirtyAttribDivisor)
        {
            updateAttribDivisor(index);
        }
    }
}

}  // rx
