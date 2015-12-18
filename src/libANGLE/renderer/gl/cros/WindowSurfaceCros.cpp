//
// Copyright (c) 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// WindowSurfaceCros.cpp: Chrome OS implementation of egl::Surface for windows

#include "libANGLE/renderer/gl/cros/WindowSurfaceCros.h"

#include <iostream>

#include "common/debug.h"

#include "libANGLE/renderer/gl/cros/DisplayDRM.h"
#include "libANGLE/renderer/gl/FramebufferGL.h"

namespace rx
{

WindowSurfaceCros::WindowSurfaceCros(RendererGL *renderer, DisplayDRM::Buffer *buffer)
    : SurfaceGL(renderer),
      mBuffer(buffer)
{
}

WindowSurfaceCros::~WindowSurfaceCros()
{
    delete mBuffer;
}

egl::Error WindowSurfaceCros::initialize()
{
    return egl::Error(EGL_SUCCESS);
}

FramebufferImpl *WindowSurfaceCros::createDefaultFramebuffer(const gl::Framebuffer::Data &data)
{
    return mBuffer->framebufferGL(data);
}

egl::Error WindowSurfaceCros::makeCurrent()
{
    return egl::Error(EGL_SUCCESS);
}

egl::Error WindowSurfaceCros::swap()
{
    mBuffer->present();
    return egl::Error(EGL_SUCCESS);
}

egl::Error WindowSurfaceCros::postSubBuffer(EGLint x, EGLint y, EGLint width, EGLint height)
{
    UNIMPLEMENTED();
    return egl::Error(EGL_SUCCESS);
}

egl::Error WindowSurfaceCros::querySurfacePointerANGLE(EGLint attribute, void **value)
{
    UNIMPLEMENTED();
    return egl::Error(EGL_SUCCESS);
}

egl::Error WindowSurfaceCros::bindTexImage(gl::Texture *texture, EGLint buffer)
{
    UNIMPLEMENTED();
    return egl::Error(EGL_SUCCESS);
}

egl::Error WindowSurfaceCros::releaseTexImage(EGLint buffer)
{
    UNIMPLEMENTED();
    return egl::Error(EGL_SUCCESS);
}

void WindowSurfaceCros::setSwapInterval(EGLint interval)
{
    mSwapControl.targetSwapInterval = interval;
}

EGLint WindowSurfaceCros::getWidth() const
{
    return mBuffer->mWidth;
}

EGLint WindowSurfaceCros::getHeight() const
{
    return mBuffer->mHeight;
}

EGLint WindowSurfaceCros::isPostSubBufferSupported() const
{
    UNIMPLEMENTED();
    return EGL_FALSE;
}

EGLint WindowSurfaceCros::getSwapBehavior() const
{
    return EGL_BUFFER_PRESERVED;
}

}
