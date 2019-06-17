//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// TestSuite_unittest.cpp: Unit tests for ANGLE's test harness.
//

#include <gtest/gtest.h>

#include "TestSuite.h"
#include "common/debug.h"
#include "util/test_utils.h"

namespace
{
// Trigger a test timeout.
TEST(TestSuiteTest, DISABLED_Timeout)
{
    angle::Sleep(30000);
}

// Trigger a test crash.
TEST(TestSuiteTest, DISABLED_Crash)
{
    ANGLE_CRASH();
}

// Trigger a test ASSERT.
TEST(TestSuiteTest, DISABLED_Assert)
{
    ASSERT(false);
}
}  // namespace

int main(int argc, char **argv)
{
    angle::TestSuite testSuite(&argc, argv);
    return testSuite.run();
}
