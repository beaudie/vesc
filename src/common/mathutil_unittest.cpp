//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// mathutil_unittests:
//   Unit tests for the utils defined in mathutil.h
//

#include "mathutil.h"

#include <gtest/gtest.h>

using namespace gl;

namespace
{

// Test the correctness of packSnorm2x16 and unpackSnorm2x16 functions.
// For floats f1 and f2, unpackSNorm2x16(packSNorm2x16(f1, f2)) should be same as f1 and f2.
TEST(MathUtilTest, packAndUnpackSnorm2x16Test)
{
    const float input[8][2] =
    {
        { 0.0f, 0.0f },
        { 1.0f, 1.0f },
        { -1.0f, 1.0f },
        { -1.0f, -1.0f },
        { 0.875f, 0.75f },
        { 0.00392f, -0.99215f },
        { -0.000675f, 0.004954f },
        { -0.6937f, -0.02146f }
    };
    const float floatFaultTolerance = 0.0001f;
    float outputVal1, outputVal2;

    for (size_t i = 0; i < 8; i++)
    {
        unpackSNorm2x16(packSNorm2x16(input[i][0], input[i][1]), &outputVal1, &outputVal2);
        EXPECT_NEAR(input[i][0], outputVal1, floatFaultTolerance);
        EXPECT_NEAR(input[i][1], outputVal2, floatFaultTolerance);
    }
}

// Test the correctness of packUnorm2x16 and unpackUnorm2x16 functions.
// For floats f1 and f2, unpackUNorm2x16(packUNorm2x16(f1, f2)) should be same as f1 and f2.
TEST(MathUtilTest, packAndUnpackUnorm2x16Test)
{
    const float input[8][2] =
    {
        { 0.0f, 0.0f },
        { 1.0f, 1.0f },
        { -1.0f, 1.0f },
        { -1.0f, -1.0f },
        { 0.875f, 0.75f },
        { 0.00392f, -0.99215f },
        { -0.000675f, 0.004954f },
        { -0.6937f, -0.02146f }
    };
    const float floatFaultTolerance = 0.0001f;
    float outputVal1, outputVal2;

    for (size_t i = 0; i < 8; i++)
    {
        unpackUNorm2x16(packUNorm2x16(input[i][0], input[i][1]), &outputVal1, &outputVal2);
        float expected = input[i][0] < 0.0f ? 0.0f : input[i][0];
        EXPECT_NEAR(expected, outputVal1, floatFaultTolerance);
        expected = input[i][1] < 0.0f ? 0.0f : input[i][1];
        EXPECT_NEAR(expected, outputVal2, floatFaultTolerance);
    }
}

// Test the correctness of packHalf2x16 and unpackHalf2x16 functions.
// For floats f1 and f2, unpackHalf2x16(packHalf2x16(f1, f2)) should be same as f1 and f2.
TEST(MathUtilTest, packAndUnpackHalf2x16Test)
{
    const float input[8][2] =
    {
        { 0.0f, 0.0f },
        { 1.0f, 1.0f },
        { -1.0f, 1.0f },
        { -1.0f, -1.0f },
        { 0.875f, 0.75f },
        { 0.00392f, -0.99215f },
        { -0.000675f, 0.004954f },
        { -0.6937f, -0.02146f },
    };
    const float floatFaultTolerance = 0.0005f;
    float outputVal1, outputVal2;

    for (size_t i = 0; i < 8; i++)
    {
        unpackHalf2x16(packHalf2x16(input[i][0], input[i][1]), &outputVal1, &outputVal2);
        EXPECT_NEAR(input[i][0], outputVal1, floatFaultTolerance);
        EXPECT_NEAR(input[i][1], outputVal2, floatFaultTolerance);
    }
}

}

