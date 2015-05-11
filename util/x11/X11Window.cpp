//
// Copyright (c) 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// X11Window.cpp: Implementation of OSWindow for X11

#include "x11/X11Window.h"

#include <iostream>

X11Window::X11Window()
    : WM_DELETE_WINDOW(None),
      mDisplay(nullptr),
      mWindow(0)
{
}

X11Window::~X11Window()
{
    destroy();
}

bool X11Window::initialize(const std::string &name, size_t width, size_t height)
{
    destroy();

    mDisplay = XOpenDisplay(NULL);
    if (!mDisplay)
    {
        return false;
    }

    {
        int screen = DefaultScreen(mDisplay);
        Window root = RootWindow(mDisplay, screen);

        Colormap colormap = XCreateColormap(mDisplay, root, DefaultVisual(mDisplay, screen), AllocNone);
        int depth = DefaultDepth(mDisplay, screen);
        Visual *visual = DefaultVisual(mDisplay, screen);

        XSetWindowAttributes attributes;
        unsigned long attributeMask = CWBorderPixel | CWColormap | CWEventMask;

        attributes.event_mask = StructureNotifyMask;
        attributes.border_pixel = 0;
        attributes.colormap = colormap;

        mWindow = XCreateWindow(mDisplay, root, 0, 0, width, height, 0, depth, InputOutput,
                                visual, attributeMask, &attributes);
        XFreeColormap(mDisplay, colormap);
    }

    if (!mWindow)
    {
        destroy();
        return false;
    }

    // Tell the window manager to notify us when the user wants to close the
    // window so we can do it ourselves.
    WM_DELETE_WINDOW = XInternAtom(mDisplay, "WM_DELETE_WINDOW", False);
    WM_PROTOCOLS = XInternAtom(mDisplay, "WM_PROTOCOLS", False);
    if (WM_DELETE_WINDOW == None || WM_PROTOCOLS == None)
    {
        destroy();
        return false;
    }

    if(XSetWMProtocols(mDisplay, mWindow, &WM_DELETE_WINDOW, 1) == 0)
    {
        destroy();
        return false;
    }

    XFlush(mDisplay);

    mX = 0;
    mY = 0;
    mWidth = width;
    mHeight = height;

    return true;
}

void X11Window::destroy()
{
    if (mWindow)
    {
        XDestroyWindow(mDisplay, mWindow);
        mWindow = 0;
    }
    if (mDisplay)
    {
        XCloseDisplay(mDisplay);
        mDisplay = nullptr;
    }
    WM_DELETE_WINDOW = None;
    WM_PROTOCOLS = None;
}

EGLNativeWindowType X11Window::getNativeWindow() const
{
    return mWindow;
}

EGLNativeDisplayType X11Window::getNativeDisplay() const
{
    return mDisplay;
}

void X11Window::messageLoop()
{
    int eventCount = XPending(mDisplay);
    while (eventCount--)
    {
        XEvent event;
        XNextEvent(mDisplay, &event);
        processEvent(event);
    }
}

void X11Window::setMousePosition(int x, int y)
{
    //TODO
}

OSWindow *CreateOSWindow()
{
    return new X11Window();
}

bool X11Window::setPosition(int x, int y)
{
    //TODO
    return true;
}

bool X11Window::resize(int width, int height)
{
    //TODO
    return true;
}

void X11Window::setVisible(bool isVisible)
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

void X11Window::signalTestEvent()
{
    //TODO
}

void X11Window::processEvent(const XEvent &event)
{
    switch (event.type)
    {
      case DestroyNotify:
        // We already received WM_DELETE_WINDOW
        break;
      case ClientMessage:
        if (event.xclient.message_type == WM_PROTOCOLS &&
            (Atom) event.xclient.data.l[0] == WM_DELETE_WINDOW)
        {
            Event event;
            event.Type = Event::EVENT_CLOSED;
            pushEvent(event);
        }
        break;
    }
}
