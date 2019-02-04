//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// angle_benchmarks_main.cpp
//   Entry point for the gtest-based benchmark tests.
//

#include <gtest/gtest.h>

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);
    testing::AddGlobalTestEnvironment(new testing::Environment());
    int rt = RUN_ALL_TESTS();
    return rt;
}
