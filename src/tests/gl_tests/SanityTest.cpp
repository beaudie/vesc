//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// SanityTest:
//   Check basic assumptions about platform availability.
//

#include "gpu_info_util/SystemInfo.h"
#include "test_utils/ANGLETest.h"

using namespace angle;

namespace
{

TEST(SanityTest, PlatformAvailabilityTest)
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

    if (IsWindows())
    {
        EXPECT_TRUE(IsPlatformAvailable(ES2_OPENGL()));
        EXPECT_TRUE(IsPlatformAvailable(ES2_OPENGLES()));
        EXPECT_TRUE(IsPlatformAvailable(ES2_VULKAN()));
    }
}

}  // anonymous namespace
