//
// Copyright (c) 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// WindowSurfaceOzone.cpp: Ozone implementation of egl::Surface for windows

#include "libANGLE/renderer/gl/egl/ozone/WindowSurfaceOzone.h"

#include "libANGLE/renderer/gl/FramebufferGL.h"
#include "libANGLE/renderer/gl/egl/ozone/DisplayOzone.h"

namespace rx
{

WindowSurfaceOzone::WindowSurfaceOzone(RendererGL *renderer, DisplayOzone::Buffer *buffer)
    : SurfaceGL(renderer), mBuffer(buffer)
{
}

WindowSurfaceOzone::~WindowSurfaceOzone()
{
    delete mBuffer;
}

egl::Error WindowSurfaceOzone::initialize()
{
    return egl::Error(EGL_SUCCESS);
}

FramebufferImpl *WindowSurfaceOzone::createDefaultFramebuffer(const gl::Framebuffer::Data &data)
{
    return mBuffer->framebufferGL(data);
}

egl::Error WindowSurfaceOzone::makeCurrent()
{
    return egl::Error(EGL_SUCCESS);
}

egl::Error WindowSurfaceOzone::swap()
{
    mBuffer->present();
    return egl::Error(EGL_SUCCESS);
}

egl::Error WindowSurfaceOzone::postSubBuffer(EGLint x, EGLint y, EGLint width, EGLint height)
{
    UNIMPLEMENTED();
    return egl::Error(EGL_SUCCESS);
}

egl::Error WindowSurfaceOzone::querySurfacePointerANGLE(EGLint attribute, void **value)
{
    UNIMPLEMENTED();
    return egl::Error(EGL_SUCCESS);
}

egl::Error WindowSurfaceOzone::bindTexImage(gl::Texture *texture, EGLint buffer)
{
    UNIMPLEMENTED();
    return egl::Error(EGL_SUCCESS);
}

egl::Error WindowSurfaceOzone::releaseTexImage(EGLint buffer)
{
    UNIMPLEMENTED();
    return egl::Error(EGL_SUCCESS);
}

void WindowSurfaceOzone::setSwapInterval(EGLint interval)
{
    mSwapControl.targetSwapInterval = interval;
}

EGLint WindowSurfaceOzone::getWidth() const
{
    return mBuffer->getWidth();
}

EGLint WindowSurfaceOzone::getHeight() const
{
    return mBuffer->getHeight();
}

EGLint WindowSurfaceOzone::isPostSubBufferSupported() const
{
    UNIMPLEMENTED();
    return EGL_FALSE;
}

EGLint WindowSurfaceOzone::getSwapBehavior() const
{
    return EGL_BUFFER_PRESERVED;
}
}  // namespace rx
