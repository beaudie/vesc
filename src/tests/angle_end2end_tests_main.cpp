//
// Copyright (c) 2013 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "gtest/gtest.h"
#include "test_utils/ANGLETest.h"

int main(int argc, char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    testing::AddGlobalTestEnvironment(new ANGLETestEnvironment());

    // If the user specified a filter, run exactly their filter.
    if (::testing::GTEST_FLAG(filter) != "*")
    {
        return RUN_ALL_TESTS();
    }

    // Run all Vulkan tests first so they can reuse the same window
    ::testing::GTEST_FLAG(filter) = "*/*VULKAN";
    int rt1 = RUN_ALL_TESTS();

    // Test window re-creation will happen here.

    // Run all other tests as they can share the same window
    ::testing::GTEST_FLAG(filter) = "-*/*VULKAN";
    int rt2 = RUN_ALL_TESTS();

    // rt is 0 on success and 1 on failure
    return std::max(rt1, rt2);
}
