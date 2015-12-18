//
// Copyright (c) 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// PbufferSurfaceCros.cpp: Chrome OS implementation of egl::Surface for PBuffers

#include "libANGLE/renderer/gl/egl/cros/PbufferSurfaceCros.h"

#include "libANGLE/renderer/gl/egl/FunctionsEGL.h"

namespace rx
{

PbufferSurfaceCros::PbufferSurfaceCros(RendererGL *renderer,
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

PbufferSurfaceCros::~PbufferSurfaceCros()
{
}

egl::Error PbufferSurfaceCros::initialize()
{
    return egl::Error(EGL_SUCCESS);
}

egl::Error PbufferSurfaceCros::makeCurrent()
{
    if (!mEGL.makeCurrent(mPbuffer, mContext))
    {
        return egl::Error(EGL_BAD_DISPLAY);
    }
    return egl::Error(EGL_SUCCESS);
}

egl::Error PbufferSurfaceCros::swap()
{
    return egl::Error(EGL_SUCCESS);
}

egl::Error PbufferSurfaceCros::postSubBuffer(EGLint x, EGLint y, EGLint width, EGLint height)
{
    return egl::Error(EGL_SUCCESS);
}

egl::Error PbufferSurfaceCros::querySurfacePointerANGLE(EGLint attribute, void **value)
{
    UNIMPLEMENTED();
    return egl::Error(EGL_SUCCESS);
}

egl::Error PbufferSurfaceCros::bindTexImage(gl::Texture *texture, EGLint buffer)
{
    UNIMPLEMENTED();
    return egl::Error(EGL_SUCCESS);
}

egl::Error PbufferSurfaceCros::releaseTexImage(EGLint buffer)
{
    UNIMPLEMENTED();
    return egl::Error(EGL_SUCCESS);
}

void PbufferSurfaceCros::setSwapInterval(EGLint interval)
{
}

EGLint PbufferSurfaceCros::getWidth() const
{
    return mWidth;
}

EGLint PbufferSurfaceCros::getHeight() const
{
    return mHeight;
}

EGLint PbufferSurfaceCros::isPostSubBufferSupported() const
{
    UNIMPLEMENTED();
    return EGL_FALSE;
}

EGLint PbufferSurfaceCros::getSwapBehavior() const
{
    return EGL_BUFFER_PRESERVED;
}

}
