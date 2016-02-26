//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// EGLTroubleshootTest.cpp:
//      tests used to troubleshoot problems in the setup tests are run.

#include <gtest/gtest.h>

#include "test_utils/ANGLETest.h"

TEST(EGLTroubleshootTest, IsRunningOnANGLE)
{
    const char *extensionString =
        static_cast<const char *>(eglQueryString(EGL_NO_DISPLAY, EGL_EXTENSIONS));
    ASSERT_NE(strstr(extensionString, "EGL_ANGLE_platform_angle"), nullptr);
}

TEST(EGLTroubleshootTest, HasGetPlatformDisplayEXT)
{
    PFNEGLGETPLATFORMDISPLAYEXTPROC eglGetPlatformDisplayEXT =
        reinterpret_cast<PFNEGLGETPLATFORMDISPLAYEXTPROC>(
            eglGetProcAddress("eglGetPlatformDisplayEXT"));

    ASSERT_NE(eglGetPlatformDisplayEXT, nullptr);
}
