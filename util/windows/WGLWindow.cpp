//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// WGLWindow:
//   Implements initializing a WGL rendering context.
//

#include "util/windows/WGLWindow.h"

WGLWindow::WGLWindow(int glesMajorVersion, int glesMinorVersion) {}

WGLWindow::~WGLWindow() {}

// Internally initializes GL resources.
bool WGLWindow::initializeGL(OSWindow *osWindow, angle::Library *wglLibrary)
{
    return true;
}

void WGLWindow::destroyGL() {}

bool WGLWindow::isGLInitialized() const
{
    return false;
}

void WGLWindow::makeCurrent() {}
