//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// EGLSyncGL.cpp: Implementation of EGL sync objects using native GL/GLES sync objects

#include "libANGLE/renderer/gl/EGLSyncGL.h"
#include "libANGLE/renderer/gl/FunctionsGL.h"
#include "libANGLE/renderer/gl/renderergl_utils.h"

namespace rx
{
EGLSyncGL::EGLSyncGL(const egl::AttributeMap &attribs, const FunctionsGL *functions)
    : mFunctions(functions), mSyncObject(nullptr)
{
    ASSERT(mFunctions);
}

EGLSyncGL::~EGLSyncGL()
{
    ASSERT(mSyncObject == 0);
}

void EGLSyncGL::onDestroy(const egl::Display *display)
{
    ASSERT(mSyncObject != 0);
    mFunctions->deleteSync(mSyncObject);
    mSyncObject = 0;
}

bool EGLSyncGL::Supported(const FunctionsGL *functions)
{
    return functions->isAtLeastGL(gl::Version(3, 2)) || functions->hasGLExtension("GL_ARB_sync") ||
           functions->isAtLeastGLES(gl::Version(3, 0));
}

egl::Error EGLSyncGL::initialize(const egl::Display *display,
                                 const gl::Context *context,
                                 EGLenum type)
{
    ASSERT(type == EGL_SYNC_FENCE_KHR);
    mSyncObject = mFunctions->fenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
    if (mSyncObject == nullptr)
    {
        return egl::Error(EGL_BAD_ALLOC, "Failed to create native GL sync object");
    }

    return egl::NoError();
}

egl::Error EGLSyncGL::clientWait(const egl::Display *display,
                                 const gl::Context *context,
                                 EGLint flags,
                                 EGLTime timeout,
                                 EGLint *outResult)
{
    ASSERT((flags & ~EGL_SYNC_FLUSH_COMMANDS_BIT_KHR) == 0);
    static_assert(EGL_SYNC_FLUSH_COMMANDS_BIT_KHR == GL_SYNC_FLUSH_COMMANDS_BIT,
                  "Unexpected enum value.");
    static_assert(EGL_FOREVER == GL_TIMEOUT_IGNORED, "Unexpected enum value.");

    ASSERT(mSyncObject != 0);
    GLenum result = mFunctions->clientWaitSync(mSyncObject, flags, timeout);
    switch (result)
    {
        case GL_TIMEOUT_EXPIRED:
            *outResult = EGL_TIMEOUT_EXPIRED;
            break;
        case GL_ALREADY_SIGNALED:
        case GL_CONDITION_SATISFIED:
            *outResult = EGL_CONDITION_SATISFIED;
            break;
        case GL_WAIT_FAILED:
            *outResult = EGL_FALSE;
            break;
        default:
            UNREACHABLE();
            break;
    }

    return egl::NoError();
}

egl::Error EGLSyncGL::serverWait(const egl::Display *display,
                                 const gl::Context *context,
                                 EGLint flags)
{
    ASSERT(mSyncObject != 0);
    ASSERT(flags == 0);
    mFunctions->waitSync(mSyncObject, flags, GL_TIMEOUT_IGNORED);
    return egl::NoError();
}

egl::Error EGLSyncGL::getStatus(const egl::Display *display, EGLint *outStatus)
{
    ASSERT(mSyncObject != 0);

    GLint status = 0;
    mFunctions->getSynciv(mSyncObject, GL_SYNC_STATUS, 1, nullptr, &status);

    switch (status)
    {
        case GL_SIGNALED:
            *outStatus = EGL_SIGNALED_KHR;
            break;
        case GL_UNSIGNALED:
            *outStatus = EGL_UNSIGNALED_KHR;
            break;
        default:
            UNREACHABLE();
            break;
    }

    return egl::NoError();
}

egl::Error EGLSyncGL::dupNativeFenceFD(const egl::Display *display, EGLint *result) const
{
    UNIMPLEMENTED();
    return egl::NoError();
}
}  // namespace rx
