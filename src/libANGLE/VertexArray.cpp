//
// Copyright (c) 2013 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Implementation of the state class for mananging GLES 3 Vertex Array Objects.
//

#include "libANGLE/VertexArray.h"
#include "libANGLE/Buffer.h"
#include "libANGLE/Context.h"
#include "libANGLE/renderer/GLImplFactory.h"
#include "libANGLE/renderer/VertexArrayImpl.h"

namespace gl
{

VertexArrayState::VertexArrayState(size_t maxAttribs, size_t maxAttribBindings)
    : mLabel(), mVertexBindings(maxAttribBindings), mMaxAttribDataSizeNoOffsetCache(maxAttribBindings, 0)
{
    ASSERT(maxAttribs <= maxAttribBindings);

    for (size_t i = 0; i < maxAttribs; i++)
    {
        mVertexAttributes.emplace_back(static_cast<GLuint>(i));
    }
}

VertexArrayState::~VertexArrayState()
{
}

void VertexArrayState::updateMaxAttribDataSizeCache(size_t index)
{
    const VertexAttribute &attrib = mVertexAttributes[index];
    const VertexBinding &binding = mVertexBindings[index];

    // FIXME: swizzle and dependent state change.
    uint64_t bufferSize = binding.getBuffer().get()->getSize();
    uint64_t attribOffset = ComputeVertexAttributeOffset(attrib, binding);
    uint64_t attribSize = ComputeVertexAttributeTypeSize(attrib);

    mMaxAttribDataSizeNoOffsetCache[index] = bufferSize - attribOffset - attribSize;
}

VertexArray::VertexArray(rx::GLImplFactory *factory,
                         GLuint id,
                         size_t maxAttribs,
                         size_t maxAttribBindings)
    : mId(id),
      mState(maxAttribs, maxAttribBindings),
      mVertexArray(factory->createVertexArray(mState))
{
}

void VertexArray::onDestroy(const Context *context)
{
    for (auto &binding : mState.mVertexBindings)
    {
        binding.setBuffer(context, nullptr);
    }
    mState.mElementArrayBuffer.set(context, nullptr);
    mVertexArray->destroy(context);
    SafeDelete(mVertexArray);
    delete this;
}

VertexArray::~VertexArray()
{
    ASSERT(!mVertexArray);
}

GLuint VertexArray::id() const
{
    return mId;
}

void VertexArray::setLabel(const std::string &label)
{
    mState.mLabel = label;
}

const std::string &VertexArray::getLabel() const
{
    return mState.mLabel;
}

void VertexArray::detachBuffer(const Context *context, GLuint bufferName)
{
    for (auto &binding : mState.mVertexBindings)
    {
        if (binding.getBuffer().id() == bufferName)
        {
            binding.setBuffer(context, nullptr);
        }
    }

    if (mState.mElementArrayBuffer.id() == bufferName)
    {
        mState.mElementArrayBuffer.set(context, nullptr);
    }
}

const VertexAttribute &VertexArray::getVertexAttribute(size_t attribIndex) const
{
    ASSERT(attribIndex < getMaxAttribs());
    return mState.mVertexAttributes[attribIndex];
}

const VertexBinding &VertexArray::getVertexBinding(size_t bindingIndex) const
{
    ASSERT(bindingIndex < getMaxBindings());
    return mState.mVertexBindings[bindingIndex];
}

size_t VertexArray::GetVertexIndexFromDirtyBit(size_t dirtyBit)
{
    static_assert(gl::MAX_VERTEX_ATTRIBS == gl::MAX_VERTEX_ATTRIB_BINDINGS,
                  "The stride of vertex attributes should equal to that of vertex bindings.");
    ASSERT(dirtyBit > DIRTY_BIT_ELEMENT_ARRAY_BUFFER);
    return (dirtyBit - DIRTY_BIT_ATTRIB_0_ENABLED) % gl::MAX_VERTEX_ATTRIBS;
}

void VertexArray::bindVertexBufferImpl(const Context *context,
                                       size_t bindingIndex,
                                       Buffer *boundBuffer,
                                       GLintptr offset,
                                       GLsizei stride)
{
    ASSERT(bindingIndex < getMaxBindings());

    VertexBinding *binding = &mState.mVertexBindings[bindingIndex];

    binding->setBuffer(context, boundBuffer);
    binding->setOffset(offset);
    binding->setStride(stride);

    // FIXME: needs swizzle.
    mState.mClientVertexArraysMask.set(bindingIndex, boundBuffer == nullptr);
    mState.mEnabledBufferArraysMask.set(bindingIndex, mState.mVertexAttributes[bindingIndex].enabled);
}

void VertexArray::bindVertexBuffer(const Context *context,
                                   size_t bindingIndex,
                                   Buffer *boundBuffer,
                                   GLintptr offset,
                                   GLsizei stride)
{
    bindVertexBufferImpl(context, bindingIndex, boundBuffer, offset, stride);

    mDirtyBits.set(DIRTY_BIT_BINDING_0_BUFFER + bindingIndex);
}

void VertexArray::setVertexAttribBinding(const Context *context,
                                         size_t attribIndex,
                                         GLuint bindingIndex)
{
    ASSERT(attribIndex < getMaxAttribs() && bindingIndex < getMaxBindings());

    if (mState.mVertexAttributes[attribIndex].bindingIndex != bindingIndex)
    {
        // In ES 3.0 contexts, the binding cannot change, hence the code below is unreachable.
        ASSERT(context->getClientVersion() >= ES_3_1);
        mState.mVertexAttributes[attribIndex].bindingIndex = bindingIndex;

        mDirtyBits.set(DIRTY_BIT_ATTRIB_0_BINDING + attribIndex);
    }
}

void VertexArray::setVertexBindingDivisor(size_t bindingIndex, GLuint divisor)
{
    ASSERT(bindingIndex < getMaxBindings());

    mState.mVertexBindings[bindingIndex].setDivisor(divisor);

    mDirtyBits.set(DIRTY_BIT_BINDING_0_DIVISOR + bindingIndex);
}

void VertexArray::setVertexAttribFormatImpl(size_t attribIndex,
                                            GLint size,
                                            GLenum type,
                                            bool normalized,
                                            bool pureInteger,
                                            GLuint relativeOffset)
{
    ASSERT(attribIndex < getMaxAttribs());

    VertexAttribute *attrib = &mState.mVertexAttributes[attribIndex];

    attrib->size           = size;
    attrib->type           = type;
    attrib->normalized     = normalized;
    attrib->pureInteger    = pureInteger;
    attrib->relativeOffset = relativeOffset;
    mState.mVertexAttributesTypeMask.setIndex(GetVertexAttributeBaseType(*attrib), attribIndex);
    mState.mEnabledAttributesMask.set(attribIndex);
}

void VertexArray::setVertexAttribFormat(size_t attribIndex,
                                        GLint size,
                                        GLenum type,
                                        bool normalized,
                                        bool pureInteger,
                                        GLuint relativeOffset)
{
    setVertexAttribFormatImpl(attribIndex, size, type, normalized, pureInteger, relativeOffset);

    mDirtyBits.set(DIRTY_BIT_ATTRIB_0_FORMAT + attribIndex);
}

void VertexArray::setVertexAttribDivisor(const Context *context, size_t attribIndex, GLuint divisor)
{
    ASSERT(attribIndex < getMaxAttribs());

    setVertexAttribBinding(context, attribIndex, static_cast<GLuint>(attribIndex));
    setVertexBindingDivisor(attribIndex, divisor);
}

void VertexArray::enableAttribute(size_t attribIndex, bool enabledState)
{
    ASSERT(attribIndex < getMaxAttribs());

    mState.mVertexAttributes[attribIndex].enabled = enabledState;
    mState.mVertexAttributesTypeMask.setIndex(
        GetVertexAttributeBaseType(mState.mVertexAttributes[attribIndex]), attribIndex);

    mDirtyBits.set(DIRTY_BIT_ATTRIB_0_ENABLED + attribIndex);

    // Update state cache
    mState.mEnabledAttributesMask.set(attribIndex, enabledState);
    mState.mEnabledBufferArraysMask.set(attribIndex, enabledState && mState.mVertexBindings[attribIndex].getBuffer().get() != nullptr);

}

void VertexArray::setVertexAttribPointer(const Context *context,
                                         size_t attribIndex,
                                         gl::Buffer *boundBuffer,
                                         GLint size,
                                         GLenum type,
                                         bool normalized,
                                         bool pureInteger,
                                         GLsizei stride,
                                         const void *pointer)
{
    ASSERT(attribIndex < getMaxAttribs());

    GLintptr offset = boundBuffer ? reinterpret_cast<GLintptr>(pointer) : 0;

    setVertexAttribFormatImpl(attribIndex, size, type, normalized, pureInteger, 0);
    setVertexAttribBinding(context, attribIndex, static_cast<GLuint>(attribIndex));

    VertexAttribute &attrib = mState.mVertexAttributes[attribIndex];

    GLsizei effectiveStride =
        stride != 0 ? stride : static_cast<GLsizei>(ComputeVertexAttributeTypeSize(attrib));
    attrib.pointer                 = pointer;
    attrib.vertexAttribArrayStride = stride;

    mState.mZeroPointerVertexArraysMask.set(attribIndex, pointer == nullptr);

    bindVertexBufferImpl(context, attribIndex, boundBuffer, offset, effectiveStride);

    mState.updateMaxAttribDataSizeCache(attribIndex);

    mDirtyBits.set(DIRTY_BIT_ATTRIB_0_POINTER + attribIndex);
}

void VertexArray::setElementArrayBuffer(const Context *context, Buffer *buffer)
{
    mState.mElementArrayBuffer.set(context, buffer);
    mDirtyBits.set(DIRTY_BIT_ELEMENT_ARRAY_BUFFER);
}

void VertexArray::syncState(const Context *context)
{
    if (mDirtyBits.any())
    {
        mVertexArray->syncState(context, mDirtyBits);
        mDirtyBits.reset();
    }
}

bool VertexArray::hasClientVertexArrays() const
{
    return (mState.mClientVertexArraysMask & mState.mEnabledAttributesMask).any();
}

bool VertexArray::hasNullPointerClientVertexArray() const
{
    return (mState.mClientVertexArraysMask & mState.mEnabledAttributesMask & mState.mZeroPointerVertexArraysMask).any();
}

AttributesMask VertexArray::getActiveBufferAttributesMask(const AttributesMask &activeAttribsMask) const
{
    return (mState.mEnabledBufferArraysMask & activeAttribsMask);
}

}  // namespace gl
