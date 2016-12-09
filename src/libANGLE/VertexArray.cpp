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
    : mLabel(), mAttribBindings(maxAttribBindings), mMaxEnabledAttribute(0)
{
    ASSERT(maxAttribs <= maxAttribBindings);

    for (size_t i = 0; i < maxAttribs; i++)
    {
        mVertexAttribs.emplace_back(static_cast<GLuint>(i), &mAttribBindings);
    }
}

VertexArrayState::~VertexArrayState()
{
    for (auto &binding : mAttribBindings)
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
    for (auto &binding : mState.mAttribBindings)
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
    ASSERT(attribIndex < getMaxVertexAttribs());
    return mState.mVertexAttribs[attribIndex];
}

const AttributeBinding &VertexArray::getAttribBinding(size_t bindingIndex) const
{
    ASSERT(bindingIndex < getMaxAttribBindings());
    return mState.mAttribBindings[bindingIndex];
}

void VertexArray::bindVertexBuffer(size_t bindingIndex,
                                   Buffer *boundBuffer,
                                   GLintptr offset,
                                   GLsizei stride)
{
    ASSERT(bindingIndex < getMaxAttribBindings());

    AttributeBinding *binding = &mState.mAttribBindings[bindingIndex];

    binding->buffer.set(boundBuffer);
    binding->offset = offset;
    binding->stride = stride;

    mDirtyBits.set(DIRTY_BIT_BINDING_0_BUFFER + bindingIndex);
}

void VertexArray::setVertexAttribBinding(size_t attribIndex, size_t bindingIndex)
{
    ASSERT(attribIndex < getMaxVertexAttribs() && bindingIndex < getMaxAttribBindings());

    AttributeFormat *format = &mState.mVertexAttribs[attribIndex].format;

    format->bindingIndex = static_cast<GLuint>(bindingIndex);

    mDirtyBits.set(DIRTY_BIT_ATTRIB_0_BINDING + attribIndex);
}

void VertexArray::setVertexBindingDivisor(size_t bindingIndex, GLuint divisor)
{
    ASSERT(bindingIndex < getMaxAttribBindings());

    mState.mAttribBindings[bindingIndex].divisor = divisor;

    mDirtyBits.set(DIRTY_BIT_BINDING_0_DIVISOR + bindingIndex);
}

void VertexArray::setVertexAttribFormat(size_t attribIndex,
                                        GLint size,
                                        GLenum type,
                                        bool normalized,
                                        bool pureInteger,
                                        GLintptr relativeOffset)
{
    ASSERT(attribIndex < getMaxVertexAttribs());

    AttributeFormat *format = &mState.mVertexAttribs[attribIndex].format;

    format->size           = size;
    format->type           = type;
    format->normalized     = normalized;
    format->pureInteger    = pureInteger;
    format->relativeOffset = relativeOffset;

    mDirtyBits.set(DIRTY_BIT_ATTRIB_0_FORMAT + attribIndex);
}

void VertexArray::setVertexAttribDivisor(size_t index, GLuint divisor)
{
    ASSERT(index < getMaxVertexAttribs());

    setVertexAttribBinding(index, index);
    setVertexBindingDivisor(index, divisor);
}

void VertexArray::enableAttribute(size_t attribIndex, bool enabledState)
{
    ASSERT(attribIndex < getMaxVertexAttribs());
    mState.mVertexAttribs[attribIndex].format.enabled = enabledState;
    mDirtyBits.set(DIRTY_BIT_ATTRIB_0_ENABLED + attribIndex);

    // Update state cache
    if (enabledState)
    {
        mState.mMaxEnabledAttribute = std::max(attribIndex + 1, mState.mMaxEnabledAttribute);
    }
    else if (mState.mMaxEnabledAttribute == attribIndex + 1)
    {
        while (mState.mMaxEnabledAttribute > 0 &&
               !mState.mVertexAttribs[mState.mMaxEnabledAttribute - 1].format.enabled)
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
    ASSERT(attribIndex < getMaxVertexAttribs());
    GLintptr offset = reinterpret_cast<GLintptr>(pointer);

    setVertexAttribFormat(attribIndex, size, type, normalized, pureInteger, 0);
    setVertexAttribBinding(attribIndex, attribIndex);

    VertexAttribute &attrib = mState.mVertexAttribs[attribIndex];
    AttributeFormat *format = &attrib.format;
    format->stride          = stride;
    format->pointer         = pointer;
    GLsizei effectiveStride =
        stride != 0 ? stride : static_cast<GLsizei>(ComputeVertexAttributeTypeSize(attrib));

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
