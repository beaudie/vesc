//
// Copyright (c) 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// SurfaceWGL.cpp: WGL implementation of egl::Surface

#include "libANGLE/renderer/gl/wgl/SurfaceWGL.h"

namespace rx
{

SurfaceWGL::SurfaceWGL(egl::Display *display, const egl::Config *config, EGLint fixedSize, EGLint postSubBufferSupported,
                       EGLenum textureFormat, EGLenum textureType)
    : SurfaceGL(display, config, fixedSize, postSubBufferSupported, textureFormat, textureType)
{
}

SurfaceWGL::~SurfaceWGL()
{
}

egl::Error SurfaceWGL::swap()
{
    return egl::Error(EGL_SUCCESS);
}

egl::Error SurfaceWGL::postSubBuffer(EGLint x, EGLint y, EGLint width, EGLint height)
{
    return egl::Error(EGL_SUCCESS);
}

egl::Error SurfaceWGL::querySurfacePointerANGLE(EGLint attribute, void **value)
{
    return egl::Error(EGL_SUCCESS);
}

egl::Error SurfaceWGL::bindTexImage(EGLint buffer)
{
    return egl::Error(EGL_SUCCESS);
}

egl::Error SurfaceWGL::releaseTexImage(EGLint buffer)
{
    return egl::Error(EGL_SUCCESS);
}

void SurfaceWGL::setSwapInterval(EGLint interval)
{
}

EGLint SurfaceWGL::getWidth() const
{
    return 0;
}

EGLint SurfaceWGL::getHeight() const
{
    return 0;
}

EGLNativeWindowType SurfaceWGL::getWindowHandle() const
{
    return NULL;
}

}
