//
// Copyright (c) 2002-2013 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// Fence.cpp: Implements the gl::FenceNV, gl::Sync and egl::Sync classes, which support the
// GL_NV_fence extension, GLES3 sync objects and EGL1.5 sync objects.

#include "libANGLE/Fence.h"

#include "angle_gl.h"

#include "common/utilities.h"
#include "libANGLE/renderer/EGLImplFactory.h"
#include "libANGLE/renderer/EGLSyncImpl.h"
#include "libANGLE/renderer/FenceNVImpl.h"
#include "libANGLE/renderer/SyncImpl.h"

namespace gl
{

FenceNV::FenceNV(rx::FenceNVImpl *impl)
    : mFence(impl), mIsSet(false), mStatus(GL_FALSE), mCondition(GL_NONE)
{}

FenceNV::~FenceNV()
{
    SafeDelete(mFence);
}

angle::Result FenceNV::set(const Context *context, GLenum condition)
{
    ANGLE_TRY(mFence->set(context, condition));

    mCondition = condition;
    mStatus    = GL_FALSE;
    mIsSet     = true;

    return angle::Result::Continue;
}

angle::Result FenceNV::test(const Context *context, GLboolean *outResult)
{
    // Flush the command buffer by default
    ANGLE_TRY(mFence->test(context, &mStatus));

    *outResult = mStatus;
    return angle::Result::Continue;
}

angle::Result FenceNV::finish(const Context *context)
{
    ASSERT(mIsSet);

    ANGLE_TRY(mFence->finish(context));

    mStatus = GL_TRUE;

    return angle::Result::Continue;
}

Sync::Sync(rx::SyncImpl *impl, GLuint id)
    : RefCountObject(id),
      mFence(impl),
      mLabel(),
      mCondition(GL_SYNC_GPU_COMMANDS_COMPLETE),
      mFlags(0)
{}

void Sync::onDestroy(const Context *context)
{
    ASSERT(mFence);
    mFence->onDestroy(context);
}

Sync::~Sync()
{
    SafeDelete(mFence);
}

void Sync::setLabel(const Context *context, const std::string &label)
{
    mLabel = label;
}

const std::string &Sync::getLabel() const
{
    return mLabel;
}

angle::Result Sync::set(const Context *context, GLenum condition, GLbitfield flags)
{
    ANGLE_TRY(mFence->set(context, condition, flags));

    mCondition = condition;
    mFlags     = flags;
    return angle::Result::Continue;
}

angle::Result Sync::clientWait(const Context *context,
                               GLbitfield flags,
                               GLuint64 timeout,
                               GLenum *outResult)
{
    ASSERT(mCondition != GL_NONE);
    return mFence->clientWait(context, flags, timeout, outResult);
}

angle::Result Sync::serverWait(const Context *context, GLbitfield flags, GLuint64 timeout)
{
    return mFence->serverWait(context, flags, timeout);
}

angle::Result Sync::getStatus(const Context *context, GLint *outResult) const
{
    return mFence->getStatus(context, outResult);
}

}  // namespace gl

namespace egl
{

Sync::Sync(rx::EGLImplFactory *factory, const AttributeMap &attribs)
    : mFence(factory->createSync(attribs)), mType(EGL_SYNC_TYPE_KHR)
{}

void Sync::onDestroy(const Display *display)
{
    ASSERT(mFence);
    mFence->onDestroy(display);
}

Sync::~Sync()
{
    SafeDelete(mFence);
}

Error Sync::set(const Display *display, EGLenum type)
{
    ANGLE_TRY(mFence->set(display, type));

    mType = type;
    return NoError();
}

Error Sync::clientWait(const Display *display, EGLint flags, EGLTime timeout, EGLint *outResult)
{
    return mFence->clientWait(display, flags, timeout, outResult);
}

Error Sync::serverWait(const Display *display, EGLint flags)
{
    return mFence->serverWait(display, flags);
}

Error Sync::getSyncAttrib(const Display *display, EGLint attribute, EGLint *value) const
{
    // Only Fence Syncs are currently supported anywhere in angle.  This function directly returns
    // attributes that are fixed for such sync objects.
    ASSERT(mType == EGL_SYNC_FENCE_KHR);

    switch (attribute)
    {
        case EGL_SYNC_TYPE_KHR:
            *value = EGL_SYNC_FENCE_KHR;
            return NoError();

        case EGL_SYNC_STATUS_KHR:
            return mFence->getStatus(display, value);

        case EGL_SYNC_CONDITION_KHR:
            *value = EGL_SYNC_PRIOR_COMMANDS_COMPLETE_KHR;
            return NoError();

        default:
            break;
    }

    UNREACHABLE();
    return NoError();
}

}  // namespace egl
