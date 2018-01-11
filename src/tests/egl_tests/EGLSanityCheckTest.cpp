//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// EGLSanityCheckTest.cpp:
//      Tests used to check environment in which other tests are run.

#include <gtest/gtest.h>

#include "gpu_info_util/SystemInfo.h"
#include "test_utils/ANGLETest.h"

// Checks the tests are running against ANGLE
TEST(EGLSanityCheckTest, IsRunningOnANGLE)
{
    const char *extensionString =
        static_cast<const char *>(eglQueryString(EGL_NO_DISPLAY, EGL_EXTENSIONS));
    ASSERT_NE(strstr(extensionString, "EGL_ANGLE_platform_angle"), nullptr);
}

// Checks that getting function pointer works
TEST(EGLSanityCheckTest, HasGetPlatformDisplayEXT)
{
    PFNEGLGETPLATFORMDISPLAYEXTPROC eglGetPlatformDisplayEXT =
        reinterpret_cast<PFNEGLGETPLATFORMDISPLAYEXTPROC>(
            eglGetProcAddress("eglGetPlatformDisplayEXT"));

    ASSERT_NE(eglGetPlatformDisplayEXT, nullptr);
}

// Checks that calling GetProcAddress for a non-existant function fails.
TEST(EGLSanityCheckTest, GetProcAddressNegativeTest)
{
    auto check = eglGetProcAddress("WigglyWombats");
    EXPECT_EQ(nullptr, check);
}

using namespace angle;

// Check basic assumptions about platform availability.
TEST(EGLSanityCheckTest, PlatformAvailabilityTest)
{
    SystemInfo info;
    GetSystemInfo(&info);

    if (IsLinux())
    {
        EXPECT_TRUE(IsPlatformAvailable(ES2_OPENGL()));
        EXPECT_TRUE(IsPlatformAvailable(ES2_VULKAN()));

        if (IsIntel(info.gpus[0].vendorId))
        {
            EXPECT_TRUE(IsPlatformAvailable(ES2_OPENGLES()));
        }
    }

    if (IsOSX())
    {
        EXPECT_TRUE(IsPlatformAvailable(ES2_OPENGL()));
        EXPECT_TRUE(IsPlatformAvailable(ES3_OPENGL()));
    }

    if (IsWindows())
    {
        EXPECT_TRUE(IsPlatformAvailable(ES2_D3D9()));
        EXPECT_TRUE(IsPlatformAvailable(ES2_D3D11()));
        EXPECT_TRUE(IsPlatformAvailable(ES3_D3D11()));
        EXPECT_TRUE(IsPlatformAvailable(ES2_OPENGL()));
        EXPECT_TRUE(IsPlatformAvailable(ES3_OPENGL()));
        EXPECT_TRUE(IsPlatformAvailable(ES2_VULKAN()));

        if (IsNvidia(info.gpus[0].vendorId))
        {
            EXPECT_TRUE(IsPlatformAvailable(ES2_OPENGLES()));
        }
    }
}
