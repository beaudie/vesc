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

class VertexArrayState final : public angle::NonCopyable
{
  public:
    VertexArrayState(size_t maxAttribs, size_t maxAttribBindings);
    ~VertexArrayState();

    const std::string &getLabel() const { return mLabel; }

    const BindingPointer<Buffer> &getElementArrayBuffer() const { return mElementArrayBuffer; }
    size_t getMaxAttribs() const { return mVertexInfos.size(); }
    size_t getMaxBindings() const { return mVertexBindings.size(); }
    size_t getMaxEnabledAttribute() const { return mMaxEnabledAttribute; }
    const std::vector<VertexInfo> &getVertexInfos() const { return mVertexInfos; }
    const VertexInfo &getVertexInfo(size_t attribIndex) const
    {
        // TODO(jiawei.shao@intel.com): Vertex Attrib Bindings
        ASSERT(mVertexInfos[attribIndex].binding ==
               &mVertexBindings[mVertexInfos[attribIndex].attrib.bindingIndex]);
        return mVertexInfos[attribIndex];
    }
    const std::vector<VertexBinding> &getVertexBindings() const { return mVertexBindings; }
    const VertexBinding &getVertexBinding(size_t bindingIndex) const
    {
        return mVertexBindings[bindingIndex];
    }

  private:
    friend class VertexArray;
    std::string mLabel;
    BindingPointer<Buffer> mElementArrayBuffer;
    std::vector<VertexBinding> mVertexBindings;
    std::vector<VertexInfo> mVertexInfos;
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

    const VertexBinding &getVertexBinding(size_t bindingIndex) const;
    const VertexInfo &getVertexInfo(size_t attribIndex) const;

    void setVertexAttribFormat(size_t attribIndex,
                               GLint size,
                               GLenum type,
                               bool normalized,
                               bool pureInteger,
                               GLintptr relativeOffset);
    void bindVertexBuffer(size_t bindingIndex,
                          Buffer *boundBuffer,
                          GLintptr offset,
                          GLsizei stride);
    void setVertexAttribBinding(size_t attribIndex, size_t bindingIndex);
    void setVertexBindingDivisor(size_t bindingIndex, GLuint divisor);

    void detachBuffer(GLuint bufferName);
    void setVertexAttribDivisor(size_t index, GLuint divisor);
    void enableAttribute(size_t attribIndex, bool enabledState);
    void setAttributeState(size_t attribIndex,
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
    size_t getMaxBindings() const { return mState.getMaxBindings(); }

    const std::vector<VertexInfo> &getVertexInfos() const { return mState.getVertexInfos(); }
    const std::vector<VertexBinding> &getVertexBindings() const
    {
        return mState.getVertexBindings();
    }

    rx::VertexArrayImpl *getImplementation() const { return mVertexArray; }

    size_t getMaxEnabledAttribute() const { return mState.getMaxEnabledAttribute(); }

    enum DirtyBitType
    {
        DIRTY_BIT_ELEMENT_ARRAY_BUFFER,

        // Reserve bits for enabled flags
        DIRTY_BIT_ATTRIB_0_ENABLED,
        DIRTY_BIT_ATTRIB_MAX_ENABLED = DIRTY_BIT_ATTRIB_0_ENABLED + gl::MAX_VERTEX_ATTRIBS,

        // Reserve bits for attrib pointers
        DIRTY_BIT_ATTRIB_0_POINTER   = DIRTY_BIT_ATTRIB_MAX_ENABLED,
        DIRTY_BIT_ATTRIB_MAX_POINTER = DIRTY_BIT_ATTRIB_0_POINTER + gl::MAX_VERTEX_ATTRIBS,

        // Reserve bits for changes to VertexAttribFormat
        DIRTY_BIT_ATTRIB_0_FORMAT   = DIRTY_BIT_ATTRIB_MAX_POINTER,
        DIRTY_BIT_ATTRIB_MAX_FORMAT = DIRTY_BIT_ATTRIB_0_FORMAT + gl::MAX_VERTEX_ATTRIBS,

        // Reserve bits for changes to VertexAttribBinding
        DIRTY_BIT_ATTRIB_0_BINDING   = DIRTY_BIT_ATTRIB_MAX_FORMAT,
        DIRTY_BIT_ATTRIB_MAX_BINDING = DIRTY_BIT_ATTRIB_0_BINDING + gl::MAX_VERTEX_ATTRIBS,

        // Reserve bits for changes to BindVertexBuffer
        DIRTY_BIT_BINDING_0_BUFFER   = DIRTY_BIT_ATTRIB_MAX_BINDING,
        DIRTY_BIT_BINDING_MAX_BUFFER = DIRTY_BIT_BINDING_0_BUFFER + gl::MAX_VERTEX_ATTRIB_BINDINGS,

        // Reserve bits for binding divisors
        DIRTY_BIT_BINDING_0_DIVISOR = DIRTY_BIT_BINDING_MAX_BUFFER,
        DIRTY_BIT_BINDING_MAX_DIVISOR =
            DIRTY_BIT_BINDING_0_DIVISOR + gl::MAX_VERTEX_ATTRIB_BINDINGS,

        DIRTY_BIT_UNKNOWN = DIRTY_BIT_BINDING_MAX_DIVISOR,
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
