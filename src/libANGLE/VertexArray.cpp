//
// Copyright (c) 2013 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Implementation of the state class for mananging GLES 3 Vertex Array Objects.
//

#include "libANGLE/VertexArray.h"
#include "libANGLE/Buffer.h"
#include "libANGLE/renderer/GLImplFactory.h"
#include "libANGLE/renderer/VertexArrayImpl.h"

namespace gl
{

VertexArrayState::VertexArrayState(size_t maxAttribs, size_t maxAttribBindings)
    : mLabel(), mAttributeBindings(maxAttribBindings), mMaxEnabledAttribute(0)
{
    ASSERT(maxAttribs <= maxAttribBindings);

    for (size_t i = 0; i < maxAttribs; i++)
    {
        mVertexAttributes.emplace_back(&mAttributeBindings[i]);
    }
}

VertexArrayState::~VertexArrayState()
{
    for (auto &binding : mAttributeBindings)
    {
        binding.buffer.set(nullptr);
    }
    mElementArrayBuffer.set(nullptr);
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

VertexArray::~VertexArray()
{
    SafeDelete(mVertexArray);
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

void VertexArray::detachBuffer(GLuint bufferName)
{
    for (auto &binding : mState.mAttributeBindings)
    {
        if (binding.buffer.id() == bufferName)
        {
            binding.buffer.set(nullptr);
        }
    }

    if (mState.mElementArrayBuffer.id() == bufferName)
    {
        mState.mElementArrayBuffer.set(nullptr);
    }
}

const VertexAttribute &VertexArray::getVertexAttribute(size_t attribIndex) const
{
    ASSERT(attribIndex < getMaxAttribs());
    return mState.mVertexAttributes[attribIndex];
}

const AttributeBinding &VertexArray::getBinding(size_t bindingIndex) const
{
    ASSERT(bindingIndex < getMaxBindings());
    return mState.mAttributeBindings[bindingIndex];
}

size_t VertexArray::GetAttribIndex(unsigned long dirtyBit)
{
    static_assert(gl::MAX_VERTEX_ATTRIBS == gl::MAX_VERTEX_ATTRIB_BINDINGS,
                  "The stride of vertex attributes should equal to that of vertex bindings.");
    return (dirtyBit - gl::VertexArray::DIRTY_BIT_ATTRIB_0_ENABLED) % gl::MAX_VERTEX_ATTRIBS;
}

void VertexArray::bindVertexBuffer(size_t bindingIndex,
                                   Buffer *boundBuffer,
                                   GLintptr offset,
                                   GLsizei stride)
{
    ASSERT(bindingIndex < getMaxBindings());

    AttributeBinding *binding = &mState.mAttributeBindings[bindingIndex];

    binding->buffer.set(boundBuffer);
    binding->offset = offset;
    binding->stride = stride;
    mDirtyBits.set(DIRTY_BIT_BINDING_0_BUFFER + bindingIndex);
}

void VertexArray::setVertexAttribBinding(size_t attribIndex, size_t bindingIndex)
{
    ASSERT(attribIndex < getMaxAttribs() && bindingIndex < getMaxBindings());

    VertexAttribute *format = &mState.mVertexAttributes[attribIndex];

    format->binding = &mState.mAttributeBindings[bindingIndex];
    mDirtyBits.set(DIRTY_BIT_ATTRIB_0_BINDING + attribIndex);
}

void VertexArray::setVertexBindingDivisor(size_t bindingIndex, GLuint divisor)
{
    ASSERT(bindingIndex < getMaxBindings());

    mState.mAttributeBindings[bindingIndex].divisor = divisor;
    mDirtyBits.set(DIRTY_BIT_BINDING_0_DIVISOR + bindingIndex);
}

void VertexArray::setVertexAttribFormat(size_t attribIndex,
                                        GLint size,
                                        GLenum type,
                                        bool normalized,
                                        bool pureInteger,
                                        GLintptr relativeOffset)
{
    ASSERT(attribIndex < getMaxAttribs());

    AttributeFormat *format = &mState.mVertexAttributes[attribIndex].format;

    format->size           = size;
    format->type           = type;
    format->normalized     = normalized;
    format->pureInteger    = pureInteger;
    format->relativeOffset = relativeOffset;
    mDirtyBits.set(DIRTY_BIT_ATTRIB_0_FORMAT + attribIndex);
}

void VertexArray::setVertexAttribDivisor(size_t index, GLuint divisor)
{
    ASSERT(index < getMaxAttribs());

    setVertexAttribBinding(index, index);
    setVertexBindingDivisor(index, divisor);
}

void VertexArray::enableAttribute(size_t attribIndex, bool enabledState)
{
    ASSERT(attribIndex < getMaxAttribs());
    mState.mVertexAttributes[attribIndex].enabled = enabledState;
    mDirtyBits.set(DIRTY_BIT_ATTRIB_0_ENABLED + attribIndex);

    // Update state cache
    if (enabledState)
    {
        mState.mMaxEnabledAttribute = std::max(attribIndex + 1, mState.mMaxEnabledAttribute);
    }
    else if (mState.mMaxEnabledAttribute == attribIndex + 1)
    {
        while (mState.mMaxEnabledAttribute > 0 &&
               !mState.mVertexAttributes[mState.mMaxEnabledAttribute - 1].enabled)
        {
            --mState.mMaxEnabledAttribute;
        }
    }
}

void VertexArray::setAttributeState(size_t attribIndex,
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

    setVertexAttribFormat(attribIndex, size, type, normalized, pureInteger, 0);
    setVertexAttribBinding(attribIndex, attribIndex);

    VertexAttribute &attrib = mState.mVertexAttributes[attribIndex];
    GLsizei effectiveStride =
        stride != 0 ? stride : static_cast<GLsizei>(ComputeVertexAttributeTypeSize(attrib));
    attrib.pointer = pointer;
    attrib.stride  = stride;

    bindVertexBuffer(attribIndex, boundBuffer, offset, effectiveStride);

    mDirtyBits.set(DIRTY_BIT_ATTRIB_0_POINTER + attribIndex);
}

void VertexArray::setElementArrayBuffer(Buffer *buffer)
{
    mState.mElementArrayBuffer.set(buffer);
    mDirtyBits.set(DIRTY_BIT_ELEMENT_ARRAY_BUFFER);
}

void VertexArray::syncImplState()
{
    if (mDirtyBits.any())
    {
        mVertexArray->syncState(mDirtyBits);
        mDirtyBits.reset();
    }
}

}  // namespace gl
