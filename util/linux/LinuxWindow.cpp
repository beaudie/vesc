//
// Copyright (c) 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "linux/LinuxWindow.h"

LinuxWindow::LinuxWindow()
    : mDisplay(nullptr),
      mWindow(0)
{
}

LinuxWindow::~LinuxWindow()
{
    destroy();
}

bool LinuxWindow::initialize(const std::string &name, size_t width, size_t height)
{
    destroy();

    mDisplay = XOpenDisplay(NULL);
    if (!mDisplay())
    {
        return false;
    }

    int screen = DefaultScreen(mDisplay);
    Window root = RootWindow(mDisplay, screen);
    colormap = XCreateColormap(mDisplay, root, DefaultVisual(mDisplay, screen), AllocNone);
    depth = DefaultDepth(mDisplay, screen);
    Visual *visual = DefaultVisual(mDisplay, screen);

    XSetWindowAttributes attributes;
    unsigned long attributeMask = CWBorderPixel | CWColormap | CWEventMask;

    // TODO(cwallez) change when input is implemented
    attributes.event_mask = 0;
    attributes.border_pixel = 0;
    attributes.colormap = mColormap;

    mWindow = XCreateWindow(mDisplay, root, 0, 0, width, height, 0, depth, InputOutput,
                            visual, attributeMask, &attributes);

    if (!mWindow)
    {
        XFreeColormap(mDisplay, colormap);
        return false;
    }

    XFlush(mDisplay);
    return true;
}

void LinuxWindow::destroy()
{
    if (mWindow)
    {
        XDestroyWindow(mDisplay, mWindow);
    }
    if (mDisplay)
    {
        XCloseDisplay(mDisplay);
        mDisplay = nullptr;
    }
}

EGLNativeWindowType LinuxWindow::getNativeWindow() const
{
    return mWindow;
}

EGLNativeDisplayType LinuxWindow::getNativeDisplay() const
{
    return mDisplay;
}

void LinuxWindow::messageLoop()
{
    //TODO
}

void LinuxWindow::setMousePosition(int x, int y)
{
    //TODO
}

OSWindow *CreateOSWindow()
{
    return new LinuxWindow();
}

bool LinuxWindow::setPosition(int x, int y)
{
    //TODO
    return true;
}

bool LinuxWindow::resize(int width, int height)
{
    //TODO
    return true;
}

void LinuxWindow::setVisible(bool isVisible)
{
    if (isVisible)
    {
        XMapWindow(mDisplay, mWindow);
    }
    else
    {
        XUnmapWindow(mDisplay, mWindow);
    }
    XFlush(mDisplay);
}

void LinuxWindow::pushEvent(Event event)
{
    //TODO
}

void LinuxWindow::signalTestEvent()
{
    //TODO
}
