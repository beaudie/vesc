//
// Copyright (c) 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// PbufferSurfaceOzone.cpp: Ozone implementation of egl::Surface for PBuffers

#include "libANGLE/renderer/gl/egl/ozone/PbufferSurfaceOzone.h"

#include "libANGLE/renderer/gl/egl/FunctionsEGL.h"

namespace rx
{

// TODO(fjhenigman) Implement this class.
PbufferSurfaceOzone::PbufferSurfaceOzone(RendererGL *renderer,
                                         EGLint width,
                                         EGLint height,
                                         bool largest,
                                         const FunctionsEGL &egl,
                                         EGLContext context,
                                         EGLConfig fbConfig)
    : SurfaceGL(renderer),
      mWidth(width),
      mHeight(height),
      mLargest(largest),
      mEGL(egl),
      mContext(context),
      mFBConfig(fbConfig),
      mPbuffer(0)
{
}

PbufferSurfaceOzone::~PbufferSurfaceOzone()
{
}

egl::Error PbufferSurfaceOzone::initialize()
{
    return egl::Error(EGL_SUCCESS);
}

egl::Error PbufferSurfaceOzone::makeCurrent()
{
    if (!mEGL.makeCurrent(mPbuffer, mContext))
    {
        return egl::Error(EGL_BAD_DISPLAY);
    }
    return egl::Error(EGL_SUCCESS);
}

egl::Error PbufferSurfaceOzone::swap()
{
    return egl::Error(EGL_SUCCESS);
}

egl::Error PbufferSurfaceOzone::postSubBuffer(EGLint x, EGLint y, EGLint width, EGLint height)
{
    return egl::Error(EGL_SUCCESS);
}

egl::Error PbufferSurfaceOzone::querySurfacePointerANGLE(EGLint attribute, void **value)
{
    UNIMPLEMENTED();
    return egl::Error(EGL_SUCCESS);
}

egl::Error PbufferSurfaceOzone::bindTexImage(gl::Texture *texture, EGLint buffer)
{
    UNIMPLEMENTED();
    return egl::Error(EGL_SUCCESS);
}

egl::Error PbufferSurfaceOzone::releaseTexImage(EGLint buffer)
{
    UNIMPLEMENTED();
    return egl::Error(EGL_SUCCESS);
}

void PbufferSurfaceOzone::setSwapInterval(EGLint interval)
{
}

EGLint PbufferSurfaceOzone::getWidth() const
{
    return mWidth;
}

EGLint PbufferSurfaceOzone::getHeight() const
{
    return mHeight;
}

EGLint PbufferSurfaceOzone::isPostSubBufferSupported() const
{
    UNIMPLEMENTED();
    return EGL_FALSE;
}

EGLint PbufferSurfaceOzone::getSwapBehavior() const
{
    return EGL_BUFFER_PRESERVED;
}
}  // namespace rx
