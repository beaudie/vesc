//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Unit tests for Program and related classes.
//

#include <gtest/gtest.h>

#include "angle_unittests_utils.h"
#include "libANGLE/Program.h"

using namespace gl;

namespace
{

// Tests that newlines get appended to the info log properly.
TEST(InfoLogTest, AppendingNewline)
{
    InfoLog infoLog;

    infoLog << "First" << 1 << 'x';
    infoLog << "Second" << 2 << 'y';

    std::string expected = "First1x\nSecond2y\n";

    EXPECT_EQ(expected, infoLog.str());
}

} // namespace
