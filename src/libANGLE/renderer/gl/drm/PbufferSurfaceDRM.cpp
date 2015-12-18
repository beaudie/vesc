//
// Copyright (c) 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// PbufferSurfaceDRM.cpp: DRM implementation of egl::Surface for PBuffers

#include "libANGLE/renderer/gl/drm/PbufferSurfaceDRM.h"

#include "common/debug.h"
#include "libANGLE/renderer/gl/drm/DisplayDRM.h"
#include "libANGLE/renderer/gl/drm/FunctionsDRM.h"

namespace rx
{

PbufferSurfaceDRM::PbufferSurfaceDRM(RendererGL *renderer,
                                     EGLint width,
                                     EGLint height,
                                     bool largest,
                                     const FunctionsDRM &drm,
                                     drm::Context context,
                                     drm::FBConfig fbConfig)
    : SurfaceGL(renderer),
      mWidth(width),
      mHeight(height),
      mLargest(largest),
      mDRM(drm),
      mContext(context),
      mFBConfig(fbConfig),
      mPbuffer(0)
{
}

PbufferSurfaceDRM::~PbufferSurfaceDRM()
{
    if (mPbuffer)
    {
        mDRM.destroyPbuffer(mPbuffer);
    }
}

egl::Error PbufferSurfaceDRM::initialize()
{
    // Avoid creating 0-sized PBuffers as it fails on the Intel Mesa driver
    // as commented on https://bugs.freedesktop.org/show_bug.cgi?id=38869 so we
    // use (w, 1) or (1, h) instead.
#ifdef XXX
    int width = std::max(1, static_cast<int>(mWidth));
    int height = std::max(1, static_cast<int>(mHeight));

    const int attribs[] =
    {
        DRM_PBUFFER_WIDTH, width,
        DRM_PBUFFER_HEIGHT, height,
        DRM_LARGEST_PBUFFER, mLargest,
        None
    };

    mPbuffer = mDRM.createPbuffer(mFBConfig, attribs);
    if (!mPbuffer)
    {
        return egl::Error(EGL_BAD_ALLOC, "Failed to create a native DRM pbuffer.");
    }

    if (mLargest)
    {
        mDRM.queryDrawable(mPbuffer, DRM_WIDTH, &mWidth);
        mDRM.queryDrawable(mPbuffer, DRM_HEIGHT, &mHeight);
    }
#endif
    return egl::Error(EGL_SUCCESS);
}

egl::Error PbufferSurfaceDRM::makeCurrent()
{
    if (mDRM.makeCurrent(mPbuffer, mContext) != True)
    {
        return egl::Error(EGL_BAD_DISPLAY);
    }
    return egl::Error(EGL_SUCCESS);
}

egl::Error PbufferSurfaceDRM::swap()
{
    return egl::Error(EGL_SUCCESS);
}

egl::Error PbufferSurfaceDRM::postSubBuffer(EGLint x, EGLint y, EGLint width, EGLint height)
{
    return egl::Error(EGL_SUCCESS);
}

egl::Error PbufferSurfaceDRM::querySurfacePointerANGLE(EGLint attribute, void **value)
{
    UNIMPLEMENTED();
    return egl::Error(EGL_SUCCESS);
}

egl::Error PbufferSurfaceDRM::bindTexImage(gl::Texture *texture, EGLint buffer)
{
    UNIMPLEMENTED();
    return egl::Error(EGL_SUCCESS);
}

egl::Error PbufferSurfaceDRM::releaseTexImage(EGLint buffer)
{
    UNIMPLEMENTED();
    return egl::Error(EGL_SUCCESS);
}

void PbufferSurfaceDRM::setSwapInterval(EGLint interval)
{
}

EGLint PbufferSurfaceDRM::getWidth() const
{
    return mWidth;
}

EGLint PbufferSurfaceDRM::getHeight() const
{
    return mHeight;
}

EGLint PbufferSurfaceDRM::isPostSubBufferSupported() const
{
    UNIMPLEMENTED();
    return EGL_FALSE;
}

EGLint PbufferSurfaceDRM::getSwapBehavior() const
{
    return EGL_BUFFER_PRESERVED;
}

}
