//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// SizedFloat_unittest:
//   Unit tests for sized float encoding/decoding.
//

#include <gtest/gtest.h>

#include "common/SizedFloat.h"

namespace angle
{
// Test our encoding code to ensure we get the values out that we expect.
// We could alternatively hardcode our inputs for these couple cases, but it's nice to do this
// programatically, since it should make it easier to write any further tests without having to
// re-encode by hand.
TEST(SizedFloatTest, Float16Encoding)
{
    EXPECT_EQ(Float16::Decode(0, 0x0f, 0), 1.0f);
    EXPECT_EQ(Float16::Decode(0, 0x0f - 1, 0), 0.5f);

    EXPECT_EQ(Float16::Assemble(0, 0x0f, 0), Float16::Encode(1.0));
    EXPECT_EQ(Float16::Assemble(0, 0x0f - 1, 0), Float16::Encode(1.0 / 2));

    EXPECT_EQ(Float16::Assemble(0, 0x0f - 3, 0), Float16::Encode(1.0 / 8));
    EXPECT_EQ(Float16::Assemble(0, 0x0f - 2, 0), Float16::Encode(2.0 / 8));
    EXPECT_EQ(Float16::Assemble(0, 0x0f - 2, 1 << (Float16::kMBits - 1)), Float16::Encode(3.0 / 8));
    EXPECT_EQ(Float16::Assemble(0, 0x0f - 1, 1 << (Float16::kMBits - 2)), Float16::Encode(5.0 / 8));
}

// Ensure our RGB9_E5 encoding is reasonable, at least for our testcase.
TEST(SizedFloatTest, RGB9E5Encoding)
{
    const auto fnTest = [](const float refR, const float refG, const float refB) {
        const auto packed = RGB9_E5::Encode(refR, refG, refB);
        float testR, testG, testB;
        RGB9_E5::Decode(packed, &testR, &testG, &testB);
        EXPECT_EQ(testR, refR);
        EXPECT_EQ(testG, refG);
        EXPECT_EQ(testB, refB);
    };
    fnTest(0.125f, 0.250f, 0.625f);
}
}  // namespace angle
