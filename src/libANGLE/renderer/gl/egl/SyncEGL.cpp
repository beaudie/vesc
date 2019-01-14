//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// SyncEGL.cpp: Implements the rx::SyncEGL class.

#include "libANGLE/renderer/gl/egl/SyncEGL.h"

#include "libANGLE/AttributeMap.h"
#include "libANGLE/Display.h"
#include "libANGLE/renderer/gl/egl/FunctionsEGL.h"

namespace rx
{

SyncEGL::SyncEGL(const egl::AttributeMap &attribs, const FunctionsEGL *egl)
    : mEGL(egl), mSync(EGL_NO_SYNC)
{}

SyncEGL::~SyncEGL()
{
    if (mSync != EGL_NO_SYNC)
    {
        mEGL->destroySyncKHR(mSync);
    }
}

egl::Error SyncEGL::set(const egl::Display *display, EGLenum type)
{
    ASSERT(type == EGL_SYNC_FENCE);
    mSync = mEGL->createSyncKHR(type, nullptr);
    if (mSync == nullptr)
    {
        return egl::Error(mEGL->getError(), "eglCreateSync failed to create sync object");
    }
    return egl::NoError();
}

egl::Error SyncEGL::clientWait(const egl::Display *display,
                               EGLint flags,
                               EGLTime timeout,
                               EGLint *outResult)
{
    ASSERT(mSync != EGL_NO_SYNC);
    EGLint result = mEGL->clientWaitSyncKHR(mSync, flags, timeout);

    if (result == EGL_FALSE)
    {
        return egl::Error(mEGL->getError(), "eglClientWaitSync failed");
    }

    *outResult = result;
    return egl::NoError();
}

egl::Error SyncEGL::serverWait(const egl::Display *display, EGLint flags)
{
    ASSERT(mSync != EGL_NO_SYNC);
    EGLint result = mEGL->waitSyncKHR(mSync, flags);

    if (result == EGL_FALSE)
    {
        return egl::Error(mEGL->getError(), "eglWaitSync failed");
    }

    return egl::NoError();
}

egl::Error SyncEGL::getStatus(const egl::Display *display, EGLint *outStatus)
{
    ASSERT(mSync != EGL_NO_SYNC);
    EGLBoolean result = mEGL->getSyncAttribKHR(mSync, EGL_SYNC_STATUS, outStatus);

    if (result == EGL_FALSE)
    {
        return egl::Error(mEGL->getError(), "eglGetSyncAttrib with EGL_SYNC_STATUS failed");
    }

    return egl::NoError();
}

void SyncEGL::destroySync(const egl::Display *display)
{
    if (mSync != EGL_NO_SYNC)
    {
        mEGL->destroySyncKHR(mSync);
        mSync = EGL_NO_SYNC;
    }
}

}  // namespace rx
