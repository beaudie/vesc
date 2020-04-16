//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// PixmapSurfaceGLX.cpp: GLX implementation of egl::Surface for Pixmaps

#include "libANGLE/renderer/gl/glx/PixmapSurfaceGLX.h"

#include "common/debug.h"
#include "libANGLE/renderer/gl/glx/DisplayGLX.h"
#include "libANGLE/renderer/gl/glx/FunctionsGLX.h"

namespace rx
{

PixmapSurfaceGLX::PixmapSurfaceGLX(const egl::SurfaceState &state,
                                   Pixmap pixmap,
                                   Display *display,
                                   const FunctionsGLX &glx,
                                   glx::FBConfig fbConfig)
    : SurfaceGLX(state),
      mWidth(0),
      mHeight(0),
      mGLX(glx),
      mFBConfig(fbConfig),
      mXPixmap(pixmap),
      mGLXPixmap(0),
      mDisplay(display)
{}

PixmapSurfaceGLX::~PixmapSurfaceGLX() {}

egl::Error PixmapSurfaceGLX::initialize(const egl::Display *display)
{
    {
        Window rootWindow;
        int x                    = 0;
        int y                    = 0;
        unsigned int borderWidth = 0;
        unsigned int depth       = 0;
        if (!XGetGeometry(mDisplay, mXPixmap, &rootWindow, &x, &y, &mWidth, &mHeight, &borderWidth,
                          &depth))
        {
            return egl::EglBadSurface() << "XGetGeometry query failed on pixmap surface.";
        }
    }

    const int attribs[] = {None};
    mGLXPixmap          = mGLX.createPixmap(mFBConfig, mXPixmap, attribs);
    if (!mGLXPixmap)
    {
        return egl::EglBadAlloc() << "Failed to create a native GLX pixmap.";
    }

    return egl::NoError();
}

egl::Error PixmapSurfaceGLX::makeCurrent(const gl::Context *context)
{
    return egl::NoError();
}

egl::Error PixmapSurfaceGLX::swap(const gl::Context *context)
{
    return egl::NoError();
}

egl::Error PixmapSurfaceGLX::postSubBuffer(const gl::Context *context,
                                           EGLint x,
                                           EGLint y,
                                           EGLint width,
                                           EGLint height)
{
    return egl::NoError();
}

egl::Error PixmapSurfaceGLX::querySurfacePointerANGLE(EGLint attribute, void **value)
{
    UNIMPLEMENTED();
    return egl::NoError();
}

egl::Error PixmapSurfaceGLX::bindTexImage(const gl::Context *context,
                                          gl::Texture *texture,
                                          EGLint buffer)
{
    UNIMPLEMENTED();
    return egl::NoError();
}

egl::Error PixmapSurfaceGLX::releaseTexImage(const gl::Context *context, EGLint buffer)
{
    UNIMPLEMENTED();
    return egl::NoError();
}

void PixmapSurfaceGLX::setSwapInterval(EGLint interval) {}

EGLint PixmapSurfaceGLX::getWidth() const
{
    return mWidth;
}

EGLint PixmapSurfaceGLX::getHeight() const
{
    return mHeight;
}

EGLint PixmapSurfaceGLX::isPostSubBufferSupported() const
{
    UNIMPLEMENTED();
    return EGL_FALSE;
}

EGLint PixmapSurfaceGLX::getSwapBehavior() const
{
    return EGL_BUFFER_PRESERVED;
}

egl::Error PixmapSurfaceGLX::checkForResize()
{
    // The size of pbuffers never change
    return egl::NoError();
}

glx::Drawable PixmapSurfaceGLX::getDrawable() const
{
    return mGLXPixmap;
}

}  // namespace rx
