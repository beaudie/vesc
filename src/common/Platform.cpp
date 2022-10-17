//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// Platform.cpp: Implementation methods for angle::Platform.

#include <platform/PlatformMethods.h>

namespace
{
// TODO(jmadill): Make methods owned by egl::Display.
angle::PlatformMethods &PlatformMethods()
{
    static angle::PlatformMethods platformMethods;
    return platformMethods;
}
}  // anonymous namespace

angle::PlatformMethods *ANGLEPlatformCurrent()
{
    return &PlatformMethods();
}
