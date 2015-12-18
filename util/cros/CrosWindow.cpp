//
// Copyright (c) 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// CrosWindow.cpp: Implementation of OSWindow for Chrome OS

#include "cros/CrosWindow.h"

int CrosWindow::sLastDepth = 0;

CrosWindow::CrosWindow()
{
}

CrosWindow::~CrosWindow()
{
}

bool CrosWindow::initialize(const std::string &name, size_t width, size_t height)
{
    mNative.x = mX = 0;
    mNative.y = mY = 0;
    mNative.width = mWidth = width;
    mNative.height = mHeight = height;
    mNative.borderWidth  = 5;
    mNative.borderHeight = 5;
    mNative.visible      = 0;
    mNative.depth        = sLastDepth++;
    return true;
}

void CrosWindow::destroy()
{
}

EGLNativeWindowType CrosWindow::getNativeWindow() const
{
    return reinterpret_cast<EGLNativeWindowType>(&mNative);
}

EGLNativeDisplayType CrosWindow::getNativeDisplay() const
{
    return EGL_DEFAULT_DISPLAY;
}

void CrosWindow::messageLoop()
{
}

void CrosWindow::setMousePosition(int x, int y)
{
}

bool CrosWindow::setPosition(int x, int y)
{
    mNative.x = mX = x;
    mNative.y = mY = y;
    return true;
}

bool CrosWindow::resize(int width, int height)
{
    mNative.width = mWidth = width;
    mNative.height = mHeight = height;
    return true;
}

void CrosWindow::setVisible(bool isVisible)
{
    mNative.visible = isVisible;
}

void CrosWindow::signalTestEvent()
{
}

OSWindow *CreateOSWindow()
{
    return new CrosWindow();
}
