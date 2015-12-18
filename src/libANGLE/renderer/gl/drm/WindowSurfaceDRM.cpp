//
// Copyright (c) 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// WindowSurfaceDRM.cpp: DRM implementation of egl::Surface for windows

#include "libANGLE/renderer/gl/drm/WindowSurfaceDRM.h"

#include "common/debug.h"

#include "libANGLE/renderer/gl/drm/DisplayDRM.h"
#include "libANGLE/renderer/gl/drm/FunctionsDRM.h"
#include "libANGLE/renderer/gl/FramebufferGL.h"
#include "libANGLE/renderer/gl/RendererGL.h"
#include "libANGLE/renderer/gl/StateManagerGL.h"

namespace rx
{

WindowSurfaceDRM::WindowSurfaceDRM(const FunctionsDRM &drm,
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
      mDRM(drm),
      mDRMDisplay(drmDisplay),
      mContext(context),
      mFBConfig(fbConfig),
      mDRMWindow(0),
      mEGLImage(eglImage),
      mFunctions(functionsGL),
      mStateManager(renderer->getStateManager()),
      mWorkarounds(renderer->getWorkarounds()),
      mFramebuffer(0)
{
}

WindowSurfaceDRM::~WindowSurfaceDRM()
{
}

egl::Error WindowSurfaceDRM::initialize()
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

FramebufferImpl *WindowSurfaceDRM::createDefaultFramebuffer(const gl::Framebuffer::Data &data)
{
    return new FramebufferGL(mFramebuffer, data, mFunctions, mWorkarounds, mStateManager);
}

egl::Error WindowSurfaceDRM::makeCurrent()
{
    if (!mDRM.makeCurrent(mDRMWindow, mContext))
    {
        return egl::Error(EGL_BAD_DISPLAY);
    }
    return egl::Error(EGL_SUCCESS);
}

egl::Error WindowSurfaceDRM::swap()
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

        mDRM.waitGL();
        XResizeWindow(mDisplay, mWindow, mParentWidth, mParentHeight);
        mDRM.waitX();
    }

    mDRMDisplay->setSwapInterval(mDRMWindow, &mSwapControl);
    mDRM.swapBuffers(mDRMWindow);
#endif

    return egl::Error(EGL_SUCCESS);
}

egl::Error WindowSurfaceDRM::postSubBuffer(EGLint x, EGLint y, EGLint width, EGLint height)
{
    UNIMPLEMENTED();
    return egl::Error(EGL_SUCCESS);
}

egl::Error WindowSurfaceDRM::querySurfacePointerANGLE(EGLint attribute, void **value)
{
    UNIMPLEMENTED();
    return egl::Error(EGL_SUCCESS);
}

egl::Error WindowSurfaceDRM::bindTexImage(gl::Texture *texture, EGLint buffer)
{
    UNIMPLEMENTED();
    return egl::Error(EGL_SUCCESS);
}

egl::Error WindowSurfaceDRM::releaseTexImage(EGLint buffer)
{
    UNIMPLEMENTED();
    return egl::Error(EGL_SUCCESS);
}

void WindowSurfaceDRM::setSwapInterval(EGLint interval)
{
    mSwapControl.targetSwapInterval = interval;
}

EGLint WindowSurfaceDRM::getWidth() const
{
    // The size of the window is always the same as the cached size of its parent.
    return mWidth;
}

EGLint WindowSurfaceDRM::getHeight() const
{
    // The size of the window is always the same as the cached size of its parent.
    return mHeight;
}

EGLint WindowSurfaceDRM::isPostSubBufferSupported() const
{
    UNIMPLEMENTED();
    return EGL_FALSE;
}

EGLint WindowSurfaceDRM::getSwapBehavior() const
{
    return EGL_BUFFER_PRESERVED;
}

}
