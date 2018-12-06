//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// WGLWindow:
//   Implements initializing a WGL rendering context.
//

#ifndef UTIL_WGLWINDOW_H_
#define UTIL_WGLWINDOW_H_

#include "export.h"
#include "common/angleutils.h"

class ANGLE_EXPORT WGLWindow : angle::NonCopyable
{
public:
    WGLWindow(int glesMajorVersion, int glesMinorVersion);
    ~WGLWindow();
};

#endif  // UTIL_WGLWINDOW_H_
