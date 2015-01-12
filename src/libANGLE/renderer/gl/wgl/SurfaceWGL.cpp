//
// Copyright (c) 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// SurfaceWGL.cpp: WGL implementation of egl::Surface

#include "libANGLE/renderer/gl/wgl/SurfaceWGL.h"

namespace rx
{

SurfaceWGL::SurfaceWGL(egl::Display *display, const egl::Config *config, EGLint width, EGLint height,
                       EGLint fixedSize, EGLint postSubBufferSupported, EGLenum textureFormat,
                       EGLenum textureType, EGLClientBuffer shareHandle)
    : SurfaceGL(display, config, width, height, fixedSize, postSubBufferSupported, textureFormat,
                textureType, shareHandle)
{
}

SurfaceWGL::~SurfaceWGL()
{
}

egl::Error SurfaceWGL::initialize()
{
    return egl::Error(EGL_SUCCESS);
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

EGLNativeWindowType SurfaceWGL::getWindowHandle() const
{
    return NULL;
}

}
