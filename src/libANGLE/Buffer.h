//
// Copyright (c) 2002-2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// Buffer.h: Defines the gl::Buffer class, representing storage of vertex and/or
// index data. Implements GL buffer objects and related functionality.
// [OpenGL ES 2.0.24] section 2.9 page 21.

#ifndef LIBANGLE_BUFFER_H_
#define LIBANGLE_BUFFER_H_

#include "common/angleutils.h"
#include "libANGLE/Debug.h"
#include "libANGLE/Error.h"
#include "libANGLE/IndexRangeCache.h"
#include "libANGLE/PackedGLEnums.h"
#include "libANGLE/RefCountObject.h"

namespace rx
{
class BufferImpl;
class GLImplFactory;
};

namespace gl
{
class Buffer;
template <bool>
class BufferTargetBinding;
class Context;

class BufferState final : angle::NonCopyable
{
  public:
    BufferState();
    ~BufferState();

    const std::string &getLabel();

    BufferUsage getUsage() const { return mUsage; }
    GLbitfield getAccessFlags() const { return mAccessFlags; }
    GLenum getAccess() const { return mAccess; }
    GLboolean isMapped() const { return mMapped; }
    void *getMapPointer() const { return mMapPointer; }
    GLint64 getMapOffset() const { return mMapOffset; }
    GLint64 getMapLength() const { return mMapLength; }
    GLint64 getSize() const { return mSize; }

  private:
    friend class Buffer;
    template <bool isTransformFeedback>
    friend class BufferTargetBinding;

    std::string mLabel;

    BufferUsage mUsage;
    GLint64 mSize;
    GLbitfield mAccessFlags;
    GLenum mAccess;
    GLboolean mMapped;
    void *mMapPointer;
    GLint64 mMapOffset;
    GLint64 mMapLength;
    std::size_t mBindingCount;
    std::size_t mTransformFeedbackBindingCount;
};

class Buffer final : public RefCountObject, public LabeledObject
{
  public:
    Buffer(rx::GLImplFactory *factory, GLuint id);
    ~Buffer() override;
    Error onDestroy(const Context *context) override;

    void setLabel(const std::string &label) override;
    const std::string &getLabel() const override;

    Error bufferData(const Context *context,
                     BufferBinding target,
                     const void *data,
                     GLsizeiptr size,
                     BufferUsage usage);
    Error bufferSubData(const Context *context,
                        BufferBinding target,
                        const void *data,
                        GLsizeiptr size,
                        GLintptr offset);
    Error copyBufferSubData(const Context *context,
                            Buffer *source,
                            GLintptr sourceOffset,
                            GLintptr destOffset,
                            GLsizeiptr size);
    Error map(const Context *context, GLenum access);
    Error mapRange(const Context *context, GLintptr offset, GLsizeiptr length, GLbitfield access);
    Error unmap(const Context *context, GLboolean *result);

    void onTransformFeedback();
    void onPixelUnpack();

    Error getIndexRange(const gl::Context *context,
                        GLenum type,
                        size_t offset,
                        size_t count,
                        bool primitiveRestartEnabled,
                        IndexRange *outRange) const;

    BufferUsage getUsage() const { return mState.mUsage; }
    GLbitfield getAccessFlags() const { return mState.mAccessFlags; }
    GLenum getAccess() const { return mState.mAccess; }
    GLboolean isMapped() const { return mState.mMapped; }
    void *getMapPointer() const { return mState.mMapPointer; }
    GLint64 getMapOffset() const { return mState.mMapOffset; }
    GLint64 getMapLength() const { return mState.mMapLength; }
    GLint64 getSize() const { return mState.mSize; }

    rx::BufferImpl *getImplementation() const { return mImpl; }

    bool isBoundForTransformFeedbackAndOtherUse() const
    {
        return mState.mTransformFeedbackBindingCount > 0 &&
               mState.mTransformFeedbackBindingCount != mState.mBindingCount;
    }

  private:
    template <bool>
    friend class BufferTargetBinding;

    BufferState mState;
    rx::BufferImpl *mImpl;

    mutable IndexRangeCache mIndexRangeCache;
};

template <bool isTransformFeedback = false>
class BufferTargetBinding : public OffsetBindingPointer<Buffer>
{
  public:
    BufferTargetBinding() : mContainerIsBound(true) {}

    explicit BufferTargetBinding(bool initiallyBound) : mContainerIsBound(initiallyBound) {}

    void set(const Context *context, Buffer *newObject) override { set(context, newObject, 0, 0); }

    void set(const Context *context, Buffer *newObject, GLintptr offset, GLsizeiptr size) override
    {
        adjustBindingCountIfBound(false);
        OffsetBindingPointer<Buffer>::set(context, newObject, offset, size);
        if (newObject)
        {
            adjustBindingCountIfBound(true);
        }
    }

    void setContainerIsBound(bool isBound)
    {
        if (mContainerIsBound == isBound)
        {
            return;
        }
        mContainerIsBound = isBound;
        adjustBindingCount(isBound);
    }

  private:
    bool mContainerIsBound;

    void adjustBindingCountIfBound(bool up)
    {
        if (mContainerIsBound)
        {
            adjustBindingCount(up);
        }
    }

    void adjustBindingCount(bool up)
    {
        if (!get())
        {
            return;
        }
        if (up)
        {
            ASSERT(get()->mState.mBindingCount !=
                   std::numeric_limits<decltype(get()->mState.mBindingCount)>::max());
            ++get()->mState.mBindingCount;
        }
        else
        {
            ASSERT(get()->mState.mBindingCount > 0);
            --get()->mState.mBindingCount;
        }
        if (isTransformFeedback)
        {
            if (up)
            {
                ASSERT(get()->mState.mTransformFeedbackBindingCount !=
                       std::numeric_limits<decltype(
                           get()->mState.mTransformFeedbackBindingCount)>::max());
                ++get()->mState.mTransformFeedbackBindingCount;
            }
            else
            {
                ASSERT(get()->mState.mTransformFeedbackBindingCount > 0);
                --get()->mState.mTransformFeedbackBindingCount;
            }
        }
    }
};

}  // namespace gl

#endif  // LIBANGLE_BUFFER_H_
