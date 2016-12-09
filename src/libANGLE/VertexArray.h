//
// Copyright (c) 2013 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// This class contains prototypes for representing GLES 3 Vertex Array Objects:
//
//   The buffer objects that are to be used by the vertex stage of the GL are collected
//   together to form a vertex array object. All state related to the definition of data used
//   by the vertex processor is encapsulated in a vertex array object.
//

#ifndef LIBANGLE_VERTEXARRAY_H_
#define LIBANGLE_VERTEXARRAY_H_

#include "libANGLE/RefCountObject.h"
#include "libANGLE/Constants.h"
#include "libANGLE/Debug.h"
#include "libANGLE/State.h"
#include "libANGLE/VertexAttribute.h"

#include <vector>

namespace rx
{
class GLImplFactory;
class VertexArrayImpl;
}  // namespace rx

namespace gl
{
class Buffer;
struct VertexBufferBinding;

class VertexArrayState final : public angle::NonCopyable
{
  public:
    explicit VertexArrayState(size_t maxAttribs, size_t maxAttribBindings);
    ~VertexArrayState();

    const std::string &getLabel() const { return mLabel; }

    const BindingPointer<Buffer> &getElementArrayBuffer() const { return mElementArrayBuffer; }
    size_t getMaxAttribs() const { return mVertexAttributes.size(); }
    size_t getMaxBufferBindings() const { return mVertexBufferBindings.size(); }
    size_t getMaxEnabledAttribute() const { return mMaxEnabledAttribute; }
    const std::vector<VertexAttribute> &getVertexAttributes() const { return mVertexAttributes; }
    const VertexAttribute &getVertexAttribute(size_t index) const
    {
        return mVertexAttributes[index];
    }
    const std::vector<VertexBufferBinding> &getVertexBufferBindings() const
    {
        return mVertexBufferBindings;
    }
    const VertexBufferBinding &getVertexBufferBinding(size_t index) const
    {
        return mVertexBufferBindings[index];
    }

  private:
    friend class VertexArray;
    std::string mLabel;
    BindingPointer<Buffer> mElementArrayBuffer;
    std::vector<VertexAttribute> mVertexAttributes;
    std::vector<VertexBufferBinding> mVertexBufferBindings;
    size_t mMaxEnabledAttribute;
};

class VertexArray final : public LabeledObject
{
  public:
    VertexArray(rx::GLImplFactory *factory, GLuint id, size_t maxAttribs, size_t maxAttribBindings);
    ~VertexArray();

    GLuint id() const;

    void setLabel(const std::string &label) override;
    const std::string &getLabel() const override;

    const VertexAttribute &getVertexAttribute(size_t attributeIndex) const;
    const VertexBufferBinding &getVertexBufferBinding(size_t bindingIndex) const;

    void setVertexAttribFormat(size_t attributeIndex,
                               GLint size,
                               GLenum type,
                               bool normalized,
                               bool pureInteger,
                               GLintptr relativeOffset);
    void bindVertexBuffer(size_t bindingIndex,
                          Buffer *boundBuffer,
                          const void *pointer,
                          GLsizei stride);
    void setVertexAttribBinding(size_t attributeIndex, size_t bindingIndex);
    void setVertexBindingDivisor(size_t bindingIndex, GLuint divisor);

    void detachBuffer(GLuint bufferName);
    void setVertexAttribDivisor(size_t index, GLuint divisor);
    void enableAttribute(size_t attributeIndex, bool enabledState);
    void setAttributeState(size_t attributeIndex,
                           Buffer *boundBuffer,
                           GLint size,
                           GLenum type,
                           bool normalized,
                           bool pureInteger,
                           GLsizei stride,
                           const void *pointer);

    void setElementArrayBuffer(Buffer *buffer);

    const BindingPointer<Buffer> &getElementArrayBuffer() const
    {
        return mState.getElementArrayBuffer();
    }
    size_t getMaxAttribs() const { return mState.getMaxAttribs(); }
    size_t getMaxBufferBindings() const { return mState.getMaxBufferBindings(); }

    const std::vector<VertexAttribute> &getVertexAttributes() const
    {
        return mState.getVertexAttributes();
    }
    const std::vector<VertexBufferBinding> &getVertexBufferBindings() const
    {
        return mState.getVertexBufferBindings();
    }

    rx::VertexArrayImpl *getImplementation() const { return mVertexArray; }

    size_t getMaxEnabledAttribute() const { return mState.getMaxEnabledAttribute(); }

    enum DirtyBitType
    {
        DIRTY_BIT_ELEMENT_ARRAY_BUFFER,

        // Reserve bits for enabled flags
        DIRTY_BIT_ATTRIB_0_ENABLED,
        DIRTY_BIT_ATTRIB_MAX_ENABLED = DIRTY_BIT_ATTRIB_0_ENABLED + gl::MAX_VERTEX_ATTRIBS,

        // Reserve bits for attrib formats
        DIRTY_BIT_ATTRIB_0_FORMAT   = DIRTY_BIT_ATTRIB_MAX_ENABLED,
        DIRTY_BIT_ATTRIB_MAX_FORMAT = DIRTY_BIT_ATTRIB_0_FORMAT + gl::MAX_VERTEX_ATTRIBS,

        // Reserve bits for vertex attrib bindings
        DIRTY_BIT_ATTRIB_0_ATTRIB_BINDING = DIRTY_BIT_ATTRIB_MAX_FORMAT,
        DIRTY_BIT_ATTRIB_MAX_ATTRIB_BINDING =
            DIRTY_BIT_ATTRIB_0_ATTRIB_BINDING + gl::MAX_VERTEX_ATTRIBS,

        // Reserve bits for vertex buffer binding points
        DIRTY_BIT_ATTRIB_0_BINDING_POINT = DIRTY_BIT_ATTRIB_MAX_ATTRIB_BINDING,
        DIRTY_BIT_ATTRIB_MAX_BINDING_POINT =
            DIRTY_BIT_ATTRIB_0_BINDING_POINT + gl::MAX_VERTEX_ATTRIB_BINDINGS,

        // Reserve bits for attrib divisors
        DIRTY_BIT_ATTRIB_0_DIVISOR   = DIRTY_BIT_ATTRIB_MAX_BINDING_POINT,
        DIRTY_BIT_ATTRIB_MAX_DIVISOR = DIRTY_BIT_ATTRIB_0_DIVISOR + gl::MAX_VERTEX_ATTRIB_BINDINGS,

        DIRTY_BIT_UNKNOWN = DIRTY_BIT_ATTRIB_MAX_DIVISOR,
        DIRTY_BIT_MAX     = DIRTY_BIT_UNKNOWN,
    };

    typedef std::bitset<DIRTY_BIT_MAX> DirtyBits;

    void syncImplState();
    bool hasAnyDirtyBit() const { return mDirtyBits.any(); }

  private:
    GLuint mId;

    VertexArrayState mState;
    DirtyBits mDirtyBits;

    rx::VertexArrayImpl *mVertexArray;
};

}  // namespace gl

#endif // LIBANGLE_VERTEXARRAY_H_
