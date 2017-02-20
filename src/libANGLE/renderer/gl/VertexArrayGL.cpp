//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// VertexArrayGL.cpp: Implements the class methods for VertexArrayGL.

#include "libANGLE/renderer/gl/VertexArrayGL.h"

#include "common/BitSetIterator.h"
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
namespace
{
// Warning: you should ensure binding really matches attrib.bindingIndex before using this function.
bool AttributeNeedsStreaming(const VertexAttribute &attrib, const VertexBinding &binding)
{
    return (attrib.enabled && binding.buffer.get() == nullptr);
}
}  // anonymous namespace

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

    // Check if the GL driver supports Vertex Attrib Binding.
    VertexAttribBindingAvailable = mFunctions->vertexAttribBinding != nullptr;

    // Set the cached vertex attribute array and vertex attribute binding array size.
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
    mStreamingElementArrayBuffer = 0;

    mStateManager->deleteBuffer(mStreamingArrayBuffer);
    mStreamingArrayBufferSize = 0;
    mStreamingArrayBuffer = 0;

    mAppliedElementArrayBuffer.set(nullptr);
    for (auto &binding : mAppliedBindings)
    {
        binding.buffer.set(nullptr);
    }
}

VertexArrayGL::AppliedAttributeGL::AppliedAttributeGL(GLuint bindingIndex)
    : enabled(false),
      type(GL_FLOAT),
      size(4u),
      normalized(false),
      pureInteger(false),
      pointer(nullptr),
      relativeOffset(0),
      bindingIndex(bindingIndex)
{
}

bool VertexArrayGL::IsAttributeUnchanged(const AppliedAttributeGL &a, const VertexAttribute &b)
{
    return a.enabled == b.enabled && a.type == b.type && a.size == b.size &&
           a.normalized == b.normalized && a.pureInteger == b.pureInteger &&
           a.pointer == b.pointer && a.relativeOffset == b.relativeOffset &&
           a.bindingIndex == b.bindingIndex;
}

bool VertexArrayGL::IsFormatUnchanged(const AppliedAttributeGL &a, const VertexAttribute &b)
{
    return a.size == b.size && a.type == b.type && a.normalized == b.normalized &&
           a.pureInteger == b.pureInteger && a.relativeOffset == b.relativeOffset;
}

bool VertexArrayGL::IsBindingBufferUnchanged(const VertexBinding &a, const VertexBinding &b)
{
    return a.buffer.get() == b.buffer.get() && a.offset == b.offset && a.stride == b.stride;
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

    // Check if any attributes need to be streamed, determines if the index range needs to be computed
    bool attributesNeedStreaming = mAttributesNeedStreaming.any();

    // Determine if an index buffer needs to be streamed and the range of vertices that need to be copied
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
        indexRange.end = first + count - 1;
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
            Error error = mData.getElementArrayBuffer()->getIndexRange(
                type, elementArrayBufferOffset, count, primitiveRestartEnabled, outIndexRange);
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

    const auto &attribs = mData.getVertexAttributes();
    const auto &bindings = mData.getVertexBindings();
    for (auto idx : angle::IterateBitSet(mAttributesNeedStreaming & activeAttributesMask))
    {
        const auto &attrib = attribs[idx];
        const auto &binding = bindings[attrib.bindingIndex];
        ASSERT(AttributeNeedsStreaming(attrib, binding));

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
    GLboolean unmapResult = GL_FALSE;
    size_t unmapRetryAttempts = 5;
    while (unmapResult != GL_TRUE && --unmapRetryAttempts > 0)
    {
        uint8_t *bufferPointer = MapBufferRangeWithFallback(mFunctions, GL_ARRAY_BUFFER, 0,
                                                            requiredBufferSize, GL_MAP_WRITE_BIT);
        size_t curBufferOffset = bufferEmptySpace;

        const auto &attribs = mData.getVertexAttributes();
        const auto &bindings = mData.getVertexBindings();
        for (auto idx : angle::IterateBitSet(mAttributesNeedStreaming & activeAttributesMask))
        {
            const auto &attrib = attribs[idx];
            const auto &binding = bindings[attrib.bindingIndex];
            ASSERT(AttributeNeedsStreaming(attrib, binding));

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
                    uint8_t *out = bufferPointer + curBufferOffset + (destStride * vertexIdx);
                    const uint8_t *in = inputPointer + sourceStride * (vertexIdx + firstIndex);
                    memcpy(out, in, destStride);
                }
            }

            // Compute where the 0-index vertex would be.
            const size_t vertexStartOffset = curBufferOffset - (firstIndex * destStride);

            if (attrib.pureInteger)
            {
                ASSERT(!attrib.normalized);
                mFunctions->vertexAttribIPointer(
                    static_cast<GLuint>(idx), attrib.size, attrib.type,
                    static_cast<GLsizei>(destStride),
                    reinterpret_cast<const GLvoid *>(vertexStartOffset));
            }
            else
            {
                mFunctions->vertexAttribPointer(
                    static_cast<GLuint>(idx), attrib.size, attrib.type, attrib.normalized,
                    static_cast<GLsizei>(destStride),
                    reinterpret_cast<const GLvoid *>(vertexStartOffset));
            }

            curBufferOffset += destStride * streamedVertexCount;

            // Mark the applied attribute as dirty by setting an invalid size so that if it doesn't
            // need to be streamed later, there is no chance that the caching will skip it.
            mAppliedAttributes[idx].size = static_cast<GLuint>(-1);
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
    const auto &attrib  = mData.getVertexAttribute(attribIndex);
    const auto &binding = mData.getBindingFromAttribIndex(attribIndex);
    mAttributesNeedStreaming.set(attribIndex, AttributeNeedsStreaming(attrib, binding));
}

void VertexArrayGL::updateAttribEnabled(size_t attribIndex)
{
    const VertexAttribute &attrib = mData.getVertexAttribute(attribIndex);
    if (mAppliedAttributes[attribIndex].enabled == attrib.enabled)
    {
        return;
    }

    updateNeedsStreaming(attribIndex);

    mStateManager->bindVertexArray(mVertexArrayID, getAppliedElementArrayBufferID());
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

bool VertexArrayGL::CanUseVertexAttribPointer(size_t attribIndex)
{
    const VertexAttribute &attrib = mData.getVertexAttribute(attribIndex);
    return attribIndex == attrib.bindingIndex && attrib.relativeOffset == 0;
}

void VertexArrayGL::updateAttribPointer(size_t attribIndex)
{
    ASSERT(CanUseVertexAttribPointer(attribIndex));

    const VertexAttribute &attrib = mData.getVertexAttribute(attribIndex);
    GLuint bindingIndex          = attrib.bindingIndex;
    const VertexBinding &binding = mData.getVertexBinding(bindingIndex);

    if (VertexArrayGL::IsAttributeUnchanged(mAppliedAttributes[attribIndex], attrib) &&
        mAppliedBindings[bindingIndex] == binding)
    {
        return;
    }

    updateNeedsStreaming(attribIndex);

    // If we need to stream, defer the attribPointer to the draw call.
    if (mAttributesNeedStreaming[attribIndex])
    {
        return;
    }

    mStateManager->bindVertexArray(mVertexArrayID, getAppliedElementArrayBufferID());
    const Buffer *arrayBuffer = binding.buffer.get();
    if (arrayBuffer != nullptr)
    {
        const BufferGL *arrayBufferGL = GetImplAs<BufferGL>(arrayBuffer);
        mStateManager->bindBuffer(GL_ARRAY_BUFFER, arrayBufferGL->getBufferID());
    }
    else
    {
        mStateManager->bindBuffer(GL_ARRAY_BUFFER, 0);
    }

    mAppliedBindings[bindingIndex].buffer = binding.buffer;

    const GLvoid *inputPointer = nullptr;
    if (arrayBuffer != nullptr)
    {
        inputPointer =
            static_cast<const GLvoid *>(reinterpret_cast<const uint8_t *>(binding.offset));
    }
    else
    {
        // Attributes using client memory ignore the VERTEX_ATTRIB_BINDING state.
        // https://www.opengl.org/registry/specs/ARB/vertex_attrib_binding
        ASSERT(binding.offset == 0);
        inputPointer = attrib.pointer;
    }

    if (attrib.pureInteger)
    {
        mFunctions->vertexAttribIPointer(static_cast<GLuint>(attribIndex), attrib.size, attrib.type,
                                         binding.stride, inputPointer);
    }
    else
    {
        mFunctions->vertexAttribPointer(static_cast<GLuint>(attribIndex), attrib.size, attrib.type,
                                        attrib.normalized, binding.stride, inputPointer);
    }
    mAppliedAttributes[attribIndex].size                    = attrib.size;
    mAppliedAttributes[attribIndex].type                    = attrib.type;
    mAppliedAttributes[attribIndex].normalized              = attrib.normalized;
    mAppliedAttributes[attribIndex].pureInteger             = attrib.pureInteger;
    mAppliedAttributes[attribIndex].pointer                 = attrib.pointer;
    mAppliedAttributes[attribIndex].relativeOffset          = attrib.relativeOffset;
    mAppliedAttributes[attribIndex].bindingIndex            = attrib.bindingIndex;

    mAppliedBindings[bindingIndex].stride = binding.stride;
    mAppliedBindings[bindingIndex].offset = binding.offset;
}

bool VertexArrayGL::CanUseVertexAttribDivisor(size_t attribIndex)
{
    return attribIndex == mData.getBindingIndexFromAttribIndex(attribIndex);
}

void VertexArrayGL::updateAttribDivisor(size_t attribIndex)
{
    ASSERT(CanUseVertexAttribDivisor(attribIndex));

    size_t bindingIndex = attribIndex;

    const VertexBinding &binding = mData.getVertexBinding(bindingIndex);
    if (mAppliedAttributes[attribIndex].bindingIndex == bindingIndex &&
        mAppliedBindings[bindingIndex].divisor == binding.divisor)
    {
        return;
    }

    mStateManager->bindVertexArray(mVertexArrayID, getAppliedElementArrayBufferID());

    mFunctions->vertexAttribDivisor(static_cast<GLuint>(attribIndex), binding.divisor);

    mAppliedAttributes[attribIndex].bindingIndex = static_cast<GLuint>(attribIndex);
    mAppliedBindings[attribIndex].divisor        = binding.divisor;
}

void VertexArrayGL::updateAttribFormat(size_t attribIndex)
{
    ASSERT(VertexAttribBindingAvailable);

    const VertexAttribute &attrib = mData.getVertexAttribute(attribIndex);
    if (VertexArrayGL::IsFormatUnchanged(mAppliedAttributes[attribIndex], attrib))
    {
        return;
    }

    mStateManager->bindVertexArray(mVertexArrayID, getAppliedElementArrayBufferID());

    if (attrib.pureInteger)
    {
        mFunctions->vertexAttribIFormat(static_cast<GLuint>(attribIndex), attrib.size, attrib.type,
                                        static_cast<GLuint>(attrib.relativeOffset));
    }
    else
    {
        mFunctions->vertexAttribFormat(static_cast<GLuint>(attribIndex), attrib.size, attrib.type,
                                       attrib.normalized,
                                       static_cast<GLuint>(attrib.relativeOffset));
    }

    mAppliedAttributes[attribIndex].size           = attrib.size;
    mAppliedAttributes[attribIndex].type           = attrib.type;
    mAppliedAttributes[attribIndex].normalized     = attrib.normalized;
    mAppliedAttributes[attribIndex].pureInteger    = attrib.pureInteger;
    mAppliedAttributes[attribIndex].relativeOffset = attrib.relativeOffset;
}

void VertexArrayGL::updateAttribBinding(size_t attribIndex)
{
    ASSERT(VertexAttribBindingAvailable);

    size_t bindingIndex = mData.getBindingIndexFromAttribIndex(attribIndex);
    if (mAppliedAttributes[attribIndex].bindingIndex == bindingIndex)
    {
        return;
    }

    mStateManager->bindVertexArray(mVertexArrayID, getAppliedElementArrayBufferID());

    mFunctions->vertexAttribBinding(static_cast<GLuint>(attribIndex),
                                    static_cast<GLuint>(bindingIndex));

    mAppliedAttributes[attribIndex].bindingIndex = static_cast<GLuint>(bindingIndex);
}

void VertexArrayGL::updateBindingBuffer(size_t bindingIndex)
{
    ASSERT(VertexAttribBindingAvailable);

    const VertexBinding &binding = mData.getVertexBinding(bindingIndex);
    if (VertexArrayGL::IsBindingBufferUnchanged(mAppliedBindings[bindingIndex], binding))
    {
        return;
    }

    mStateManager->bindVertexArray(mVertexArrayID, getAppliedElementArrayBufferID());

    GLuint bufferID           = 0;
    const Buffer *arrayBuffer = binding.buffer.get();
    if (arrayBuffer != nullptr)
    {
        bufferID = GetImplAs<BufferGL>(arrayBuffer)->getBufferID();
    }
    mFunctions->bindVertexBuffer(static_cast<GLuint>(bindingIndex), bufferID, binding.offset,
                                 binding.stride);

    mAppliedBindings[bindingIndex].buffer = binding.buffer;
    mAppliedBindings[bindingIndex].offset = binding.offset;
    mAppliedBindings[bindingIndex].stride = binding.stride;
}

void VertexArrayGL::updateBindingDivisor(size_t bindingIndex)
{
    ASSERT(VertexAttribBindingAvailable);

    GLuint divisor = mData.getVertexBinding(bindingIndex).divisor;
    if (mAppliedBindings[bindingIndex].divisor == divisor)
    {
        return;
    }

    mStateManager->bindVertexArray(mVertexArrayID, getAppliedElementArrayBufferID());

    mFunctions->vertexBindingDivisor(static_cast<GLuint>(bindingIndex), divisor);

    mAppliedBindings[bindingIndex].divisor = divisor;
}

void VertexArrayGL::syncState(const VertexArray::DirtyBits &dirtyBits)
{
    VertexArray::DirtyBits appliedDirtyBits;
    for (unsigned long dirtyBit : angle::IterateBitSet(dirtyBits))
    {
        if (dirtyBit == VertexArray::DIRTY_BIT_ELEMENT_ARRAY_BUFFER)
        {
            // TODO(jmadill): Element array buffer bindings
            continue;
        }

        size_t index = VertexArray::GetAttribIndex(dirtyBit);
        if (dirtyBit >= VertexArray::DIRTY_BIT_ATTRIB_0_ENABLED &&
            dirtyBit < VertexArray::DIRTY_BIT_ATTRIB_MAX_ENABLED)
        {
            updateAttribEnabled(index);
            continue;
        }

        if (dirtyBit >= VertexArray::DIRTY_BIT_ATTRIB_0_POINTER &&
            dirtyBit < VertexArray::DIRTY_BIT_ATTRIB_MAX_POINTER)
        {
            if (CanUseVertexAttribPointer(index))
            {
                updateAttribPointer(index);

                // According to the definition of VertexAttrib*Pointer in ES3.1 SPEC,
                // VertexAttrib*Pointer are equal to:
                //
                // if (the default vertex array object is bound and no buffer is bound to
                // ARRAY_BUFFER) {
                //     vertex_buffer = temporary buffer
                //     offset = 0;
                // } else {
                //     vertex_buffer = <buffer bound to ARRAY_BUFFER>
                //     offset = (char *)pointer - (char *)NULL;
                // }
                // VertexAttrib*Format(index, size, type, fnormalized, {normalized, }, 0);
                // VertexAttribBinding(index, index);
                // if (stride != 0) {
                // effectiveStride = stride;
                // } else {
                //     compute effectiveStride based on size and type;
                //}
                // VERTEX_ATTRIB_ARRAY_STRIDE[index] = stride;
                // VERTEX_ATTRIB_ARRAY_POINTER[index] = pointer;
                //
                // BindVertexBuffer(index, vertex_buffer, offset, effectiveStride);
                //
                // So VertexAttrib*Format, VertexAttribBinding or BindVertexBuffer are not needed to
                // call again after VertexAttrib*Pointer.
                appliedDirtyBits.set(VertexArray::DIRTY_BIT_ATTRIB_0_BINDING + index);
                appliedDirtyBits.set(VertexArray::DIRTY_BIT_ATTRIB_0_FORMAT + index);
                appliedDirtyBits.set(VertexArray::DIRTY_BIT_BINDING_0_BUFFER + index);
            }
        }
        else if (dirtyBit >= VertexArray::DIRTY_BIT_ATTRIB_0_BINDING &&
                 dirtyBit < VertexArray::DIRTY_BIT_ATTRIB_MAX_BINDING)
        {
            // VertexAttribBinding isn't needed after VertexAttrib*Pointer.
            if (appliedDirtyBits.test(dirtyBit))
            {
                ASSERT(CanUseVertexAttribDivisor(index));

                // We can only use VertexAttribDivisor without Vertex Attrib Binding,
                // or we should use VertexBindingDivisor instead.
                if (dirtyBits.test(VertexArray::DIRTY_BIT_BINDING_0_DIVISOR + index) &&
                    !VertexAttribBindingAvailable)
                {
                    updateAttribDivisor(index);

                    // VertexBindingDivisor isn't needed after VertexAttribDivisor.
                    appliedDirtyBits.set(VertexArray::DIRTY_BIT_BINDING_0_DIVISOR + index);
                }
            }
            // According to the definition of VertexAttribDivisor in ES3.1 SPEC, VertexAttribDivisor
            // is equal to:
            //
            // VertexAttribBinding(index, index);
            // VertexBindingDivisor(index, divisor);
            //
            // Therefore, VertexAttribBinding isn't needed when VertexAttribDivisor will be called.
            else if (CanUseVertexAttribDivisor(index) &&
                     dirtyBits.test(VertexArray::DIRTY_BIT_BINDING_0_DIVISOR + index))
            {
                updateAttribDivisor(index);

                // VertexBindingDivisor isn't needed after VertexAttribDivisor.
                appliedDirtyBits.set(VertexArray::DIRTY_BIT_BINDING_0_DIVISOR + index);
            }
            else
            {
                updateAttribBinding(index);
            }
        }
        else if (dirtyBit >= VertexArray::DIRTY_BIT_ATTRIB_0_FORMAT &&
                 dirtyBit < VertexArray::DIRTY_BIT_ATTRIB_MAX_FORMAT)
        {
            // VertexAttribFormat isn't needed after VertexAttrib*Pointer.
            if (appliedDirtyBits.test(dirtyBit))
            {
                ASSERT(CanUseVertexAttribPointer(index));
            }
            else
            {
                updateAttribFormat(index);
            }
        }
        else if (dirtyBit >= VertexArray::DIRTY_BIT_BINDING_0_BUFFER &&
                 dirtyBit < VertexArray::DIRTY_BIT_BINDING_MAX_BUFFER)
        {
            // BindVertexBuffer isn't needed after VertexAttrib*Pointer.
            if (appliedDirtyBits.test(dirtyBit))
            {
                ASSERT(CanUseVertexAttribPointer(index));
            }
            else
            {
                updateBindingBuffer(index);
            }
        }
        else if (dirtyBit >= VertexArray::DIRTY_BIT_BINDING_0_DIVISOR &&
                 dirtyBit < VertexArray::DIRTY_BIT_BINDING_MAX_DIVISOR)
        {
            // VertexBindingDivisor isn't needed after VertexAttribDivisor.
            if (appliedDirtyBits.test(dirtyBit))
            {
                ASSERT(CanUseVertexAttribDivisor(index));
            }
            else
            {
                updateBindingDivisor(index);
            }
        }
        else
            UNREACHABLE();
    }
}

}  // rx
