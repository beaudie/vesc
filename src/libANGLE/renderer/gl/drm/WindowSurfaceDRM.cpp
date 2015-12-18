//
// Copyright (c) 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// WindowSurfaceDRM.cpp: DRM implementation of egl::Surface for windows

#include "libANGLE/renderer/gl/drm/WindowSurfaceDRM.h"

#include "common/debug.h"

#include "libANGLE/renderer/gl/drm/DisplayDRM.h"
#include "libANGLE/renderer/gl/drm/FunctionsDRM.h"

namespace rx
{

WindowSurfaceDRM::WindowSurfaceDRM(const FunctionsDRM &drm,
                                   DisplayDRM *drmDisplay,
                                   RendererGL *renderer,
                                   Window window,
                                   EGLDisplay display,
                                   drm::Context context,
                                   drm::FBConfig fbConfig)
    : SurfaceGL(renderer),
      mParent(window),
      mWindow(0),
      mDisplay(display),
      mDRM(drm),
      mDRMDisplay(drmDisplay),
      mContext(context),
      mFBConfig(fbConfig),
      mDRMWindow(0)
{
}

WindowSurfaceDRM::~WindowSurfaceDRM()
{
    if (mDRMWindow)
    {
        mDRM.destroyWindow(mDRMWindow);
    }

    if (mWindow)
    {
        //XXX XDestroyWindow(mDisplay, mWindow);
    }

    mDRMDisplay->syncXCommands();
}

egl::Error WindowSurfaceDRM::initialize()
{
#ifdef XXX
    // Check that the window's visual ID is valid, as part of the AMGLE_x11_visual
    // extension.
    {
        XWindowAttributes windowAttributes;
        XGetWindowAttributes(mDisplay, mParent, &windowAttributes);
        int visualId = windowAttributes.visual->visualid;

        if (!mDRMDisplay->isValidWindowVisualId(visualId))
        {
            return egl::Error(EGL_BAD_MATCH,
                              "The visual of native_window doesn't match the visual given with "
                              "ANGLE_X11_VISUAL_ID");
        }
    }

    // The visual of the X window, DRM window and DRM context must match,
    // however we received a user-created window that can have any visual
    // and wouldn't work with our DRM context. To work in all cases, we
    // create a child window with the right visual that covers all of its
    // parent.
    XVisualInfo *visualInfo = mDRM.getVisualFromFBConfig(mFBConfig);
    if (!visualInfo)
    {
        return egl::Error(EGL_BAD_NATIVE_WINDOW, "Failed to get the XVisualInfo for the child window.");
    }
    Visual* visual = visualInfo->visual;

    if (!getWindowDimensions(mParent, &mParentWidth, &mParentHeight))
    {
        return egl::Error(EGL_BAD_NATIVE_WINDOW, "Failed to get the parent window's dimensions.");
    }

    // The depth, colormap and visual must match otherwise we get a X error
    // so we specify the colormap attribute. Also we do not want the window
    // to be taken into account for input so we specify the event and
    // do-not-propagate masks to 0 (the defaults). Finally we specify the
    // border pixel attribute so that we can use a different visual depth
    // than our parent (seems like X uses that as a condition to render
    // the subwindow in a different buffer)
    XSetWindowAttributes attributes;
    unsigned long attributeMask = CWColormap | CWBorderPixel;

    Colormap colormap XCreateColormap(mDisplay, mParent, visual, AllocNone);
    if(!colormap)
    {
        XFree(visualInfo);
        return egl::Error(EGL_BAD_NATIVE_WINDOW, "Failed to create the Colormap for the child window.");
    }
    attributes.colormap = colormap;
    attributes.border_pixel = 0;

    //TODO(cwallez) set up our own error handler to see if the call failed
    mWindow = XCreateWindow(mDisplay, mParent, 0, 0, mParentWidth, mParentHeight, 0, visualInfo->depth, InputOutput, visual, attributeMask, &attributes);

    //XXX this would call glXCreateWindow.  we don't need to call eglCreateSurface or whatever it's called
    mDRMWindow = mDRM.createWindow(mFBConfig, mWindow, nullptr);

    XMapWindow(mDisplay, mWindow);
    XFlush(mDisplay);

    XFree(visualInfo);
    XFreeColormap(mDisplay, colormap);

    mDRMDisplay->syncXCommands();
#endif

    return egl::Error(EGL_SUCCESS);
}

egl::Error WindowSurfaceDRM::makeCurrent()
{
    if (mDRM.makeCurrent(mDRMWindow, mContext) != True)
    {
        return egl::Error(EGL_BAD_DISPLAY);
    }
    return egl::Error(EGL_SUCCESS);
}

egl::Error WindowSurfaceDRM::swap()
{
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
        //XXX XResizeWindow(mDisplay, mWindow, mParentWidth, mParentHeight);
        mDRM.waitX();
    }

    mDRMDisplay->setSwapInterval(mDRMWindow, &mSwapControl);
    mDRM.swapBuffers(mDRMWindow);

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
    return mParentWidth;
}

EGLint WindowSurfaceDRM::getHeight() const
{
    // The size of the window is always the same as the cached size of its parent.
    return mParentHeight;
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

bool WindowSurfaceDRM::getWindowDimensions(Window window, unsigned int *width, unsigned int *height) const
{
#ifdef XXX
    Window root;
    int x, y;
    unsigned int border, depth;
    XGetGeometry(mDisplay, window, &root, &x, &y, width, height, &border, &depth) != 0;
#else
    *width = *height = 10;
    return 1;
#endif
}

}
