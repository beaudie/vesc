//
// Copyright (c) 2002-2013 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// Fence.h: Defines the gl::FenceNV and gl::Sync classes, which support the GL_NV_fence
// extension and GLES3 sync objects.

#ifndef LIBANGLE_FENCE_H_
#define LIBANGLE_FENCE_H_

#include "libANGLE/Debug.h"
#include "libANGLE/Error.h"
#include "libANGLE/RefCountObject.h"

#include "common/angleutils.h"

namespace rx
{
class EGLImplFactory;
class EglSyncImpl;
class FenceNVImpl;
class SyncImpl;
}  // namespace rx

namespace gl
{

class FenceNV final : angle::NonCopyable
{
  public:
    explicit FenceNV(rx::FenceNVImpl *impl);
    virtual ~FenceNV();

    angle::Result set(const Context *context, GLenum condition);
    angle::Result test(const Context *context, GLboolean *outResult);
    angle::Result finish(const Context *context);

    bool isSet() const { return mIsSet; }
    GLboolean getStatus() const { return mStatus; }
    GLenum getCondition() const { return mCondition; }

  private:
    rx::FenceNVImpl *mFence;

    bool mIsSet;

    GLboolean mStatus;
    GLenum mCondition;
};

class Sync final : public RefCountObject, public LabeledObject
{
  public:
    Sync(rx::SyncImpl *impl, GLuint id);
    ~Sync() override;

    void onDestroy(const Context *context) override;

    void setLabel(const Context *context, const std::string &label) override;
    const std::string &getLabel() const override;

    angle::Result set(const Context *context, GLenum condition, GLbitfield flags);
    angle::Result clientWait(const Context *context,
                             GLbitfield flags,
                             GLuint64 timeout,
                             GLenum *outResult);
    angle::Result serverWait(const Context *context, GLbitfield flags, GLuint64 timeout);
    angle::Result getStatus(const Context *context, GLint *outResult) const;

    GLenum getCondition() const { return mCondition; }
    GLbitfield getFlags() const { return mFlags; }

  private:
    rx::SyncImpl *mFence;

    std::string mLabel;

    GLenum mCondition;
    GLbitfield mFlags;
};

}  // namespace gl

namespace egl
{
class Sync final : public angle::RefCountObject<Display, angle::Result>
{
  public:
    Sync(rx::EGLImplFactory *factory, const AttributeMap &attribs);
    ~Sync() override;

    void onDestroy(const Display *display) override;

    egl::Error set(const Display *display, EGLenum type);
    egl::Error clientWait(const Display *display, EGLint flags, EGLTime timeout, EGLint *outResult);
    egl::Error serverWait(const Display *display, EGLint flags);
    egl::Error getSyncAttrib(const Display *display, EGLint attribute, EGLAttrib *value) const;

    // Used to notify that eglDestroySync is called, which causes an implicit signal on the fence.
    // Note that the actual Sync object can still be referenced, so this does not have the same
    // semantics as onDestroy().
    void destroySync(const Display *display);

    EGLenum getType() const { return mType; }

  private:
    rx::EglSyncImpl *mFence;

    EGLenum mType;
};

}  // namespace egl

#endif  // LIBANGLE_FENCE_H_
