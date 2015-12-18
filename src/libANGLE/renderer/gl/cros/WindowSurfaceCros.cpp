//
// Copyright (c) 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// WindowSurfaceCros.cpp: Chrome OS implementation of egl::Surface for windows

#include "libANGLE/renderer/gl/cros/WindowSurfaceCros.h"

#include "common/debug.h"

#include "libANGLE/renderer/gl/cros/DisplayDRM.h"
#include "libANGLE/renderer/gl/cros/FunctionsEGL.h"
#include "libANGLE/renderer/gl/FramebufferGL.h"
#include "libANGLE/renderer/gl/RendererGL.h"
#include "libANGLE/renderer/gl/StateManagerGL.h"

namespace rx
{

WindowSurfaceCros::WindowSurfaceCros(const FunctionsEGL &egl,
                                   DisplayDRM *drmDisplay,
                                   RendererGL *renderer,
                                   int width,
                                   int height,
                                   EGLImageKHR eglImage,
                                   const FunctionsGL *functionsGL,
                                   EGLDisplay display,
                                   EGLContext context,
                                   EGLConfig fbConfig)
    : SurfaceGL(renderer),
      mWidth(width),
      mHeight(height),
      mDisplay(display),
      mEGL(egl),
      mDRMDisplay(drmDisplay),
      mContext(context),
      mFBConfig(fbConfig),
      mEGLImage(eglImage),
      mFunctions(functionsGL),
      mStateManager(renderer->getStateManager()),
      mWorkarounds(renderer->getWorkarounds()),
      mFramebuffer(0)
{
}

WindowSurfaceCros::~WindowSurfaceCros()
{
}

egl::Error WindowSurfaceCros::initialize()
{
    GLuint renderbuffer;

    mFunctions->genRenderbuffers(1, &renderbuffer);
    mStateManager->bindRenderbuffer(GL_RENDERBUFFER, renderbuffer);
    mFunctions->eglImageTargetRenderbufferStorageOES(GL_RENDERBUFFER, mEGLImage);

    mFunctions->genFramebuffers(1, &mFramebuffer);
    mStateManager->bindFramebuffer(GL_FRAMEBUFFER, mFramebuffer);

    mFunctions->framebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0_EXT, GL_RENDERBUFFER, renderbuffer);

    bool depth = true, stencil = true;
    GLuint depth_stencil_format = GL_DEPTH24_STENCIL8_OES;
    GLuint depth_stencil;

    if (depth || stencil)
    {
        mFunctions->genRenderbuffers(1, &depth_stencil);
        mStateManager->bindRenderbuffer(GL_RENDERBUFFER, depth_stencil);
        mFunctions->renderbufferStorage(GL_RENDERBUFFER, depth_stencil_format, mWidth, mHeight);
    }

    if (depth)
    {
        mFunctions->framebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_stencil);
    }

    if (stencil)
    {
        mFunctions->framebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, depth_stencil);
    }

    return egl::Error(EGL_SUCCESS);
}

FramebufferImpl *WindowSurfaceCros::createDefaultFramebuffer(const gl::Framebuffer::Data &data)
{
    return new FramebufferGL(mFramebuffer, data, mFunctions, mWorkarounds, mStateManager);
}

egl::Error WindowSurfaceCros::makeCurrent()
{
    return egl::Error(EGL_SUCCESS);
}

egl::Error WindowSurfaceCros::swap()
{
#ifdef XXX
    //TODO(cwallez) set up our own error handler to see if the call failed
    unsigned int newParentWidth, newParentHeight;
    if (!getWindowDimensions(mParent, &newParentWidth, &newParentHeight))
    {
        // TODO(cwallez) What error type here?
        return egl::Error(EGL_BAD_CURRENT_SURFACE, "Failed to retrieve the size of the parent window.");
    }

    if (mParentWidth != newParentWidth || mParentHeight != newParentHeight)
    {
        mParentWidth = newParentWidth;
        mParentHeight = newParentHeight;

        mEGL.waitGL();
        XResizeWindow(mDisplay, mWindow, mParentWidth, mParentHeight);
        mEGL.waitX();
    }

    mDRMDisplay->setSwapInterval(mEGLindow, &mSwapControl);
    mEGL.swapBuffers(mEGLindow);
#endif

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
    // The size of the window is always the same as the cached size of its parent.
    return mWidth;
}

EGLint WindowSurfaceCros::getHeight() const
{
    // The size of the window is always the same as the cached size of its parent.
    return mHeight;
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
