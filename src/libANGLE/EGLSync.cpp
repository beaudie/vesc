//
// Copyright (c) 2002-2013 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// EGLSync.cpp: Implements the egl::Sync class.

#include "libANGLE/EGLSync.h"

#include "angle_gl.h"

#include "common/utilities.h"
#include "libANGLE/renderer/EGLImplFactory.h"
#include "libANGLE/renderer/EGLSyncImpl.h"

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
