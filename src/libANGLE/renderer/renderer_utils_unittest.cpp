//
// Copyright (c) 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// renderer_utils_unittest:
//   Unit tests for utility functions in renderer_utils.

#include <gtest/gtest.h>

#include "libANGLE/renderer/renderer_utils.h"

namespace angle
{
// Test rectangles are not clipped when they fit in the bounds
TEST(ClipBlitRectangles, NoClipping)
{
    const gl::Rectangle original(0, 0, 100, 100);
    const gl::Rectangle slightlyBigger(0, 0, 101, 101);
    gl::Extents size(100, 100, 1);

    {
        gl::Rectangle a(original);
        gl::Rectangle b(original);
        EXPECT_TRUE(rx::ClipBlitRectangles(&a, &b, size));
        EXPECT_EQ(original, a);
        EXPECT_EQ(original, b);
    }

    {
        gl::Rectangle a(original);
        gl::Rectangle b(slightlyBigger);
        EXPECT_TRUE(rx::ClipBlitRectangles(&a, &b, size));
        EXPECT_EQ(original, a);
        EXPECT_EQ(slightlyBigger, b);
    }
}

// Test rectangles are clipped correctly when they have positive sizes
TEST(ClipBlitRectangles, TooLargePositiveSizes)
{
    gl::Rectangle source(0, 0, 100, 100);
    gl::Rectangle dest(0, 0, 50, 50);
    gl::Extents size(50, 50, 1);
    EXPECT_TRUE(rx::ClipBlitRectangles(&source, &dest, size));
    EXPECT_EQ(gl::Rectangle(0, 0, 50, 50), source);
    EXPECT_EQ(gl::Rectangle(0, 0, 25, 25), dest);
}

// Test rectangles are clipped correctly when they have negative sizes
TEST(ClipBlitRectangles, TooLargeNegativeSizes)
{
    gl::Rectangle source(100, 100, -100, -100);
    gl::Rectangle dest(50, 50, -50, -50);
    gl::Extents size(50, 50, 1);
    EXPECT_TRUE(rx::ClipBlitRectangles(&source, &dest, size));
    EXPECT_EQ(gl::Rectangle(50, 50, -50, -50), source);
    EXPECT_EQ(gl::Rectangle(25, 25, -25, -25), dest);
}

// Test rectangles are clipped correctly when they have negative sizes
TEST(ClipBlitRectangles, TooLargeInvertedSizes)
{
    gl::Rectangle source(100, 100, -100, -100);
    gl::Rectangle dest(0, 0, 50, 50);
    gl::Extents size(50, 50, 1);
    EXPECT_TRUE(rx::ClipBlitRectangles(&source, &dest, size));
    EXPECT_EQ(gl::Rectangle(50, 50, -50, -50), source);
    EXPECT_EQ(gl::Rectangle(25, 25, 25, 25), dest);
}

// Test rectangles are clipped correctly when they underflow
TEST(ClipBlitRectangles, TooSmallPositiveSizes)
{
    gl::Rectangle source(-50, -50, 100, 100);
    gl::Rectangle dest(0, 0, 50, 50);
    gl::Extents size(50, 50, 1);
    EXPECT_TRUE(rx::ClipBlitRectangles(&source, &dest, size));
    EXPECT_EQ(gl::Rectangle(0, 0, 50, 50), source);
    EXPECT_EQ(gl::Rectangle(25, 25, 25, 25), dest);
}

// Test rectangles are clipped correctly when they underflow
TEST(ClipBlitRectangles, TooSmallNegativeSizes)
{
    gl::Rectangle source(25, 25, -50, -50);
    gl::Rectangle dest(0, 0, 50, 50);
    gl::Extents size(50, 50, 1);
    EXPECT_TRUE(rx::ClipBlitRectangles(&source, &dest, size));
    EXPECT_EQ(gl::Rectangle(25, 25, -25, -25), source);
    EXPECT_EQ(gl::Rectangle(0, 0, 25, 25), dest);
}

// Test rectangles are clipped correctly when they underflow and overflow
TEST(ClipBlitRectangles, TooLargeAndTooSmall)
{
    gl::Rectangle source(-50, -50, 200, 200);
    gl::Rectangle dest(0, 0, 100, 100);
    gl::Extents size(50, 50, 1);
    EXPECT_TRUE(rx::ClipBlitRectangles(&source, &dest, size));
    EXPECT_EQ(gl::Rectangle(0, 0, 50, 50), source);
    EXPECT_EQ(gl::Rectangle(75, 75, 25, 25), dest);
}

// Test that ClipBlitRectangles retuns false when the source rectangle does not intersect with the
// framebuffer
TEST(ClipBlitRectangles, OutsideBounds)
{
    // Both dimensions out of bounds
    {
        gl::Rectangle source(100, 100, 50, 50);
        gl::Rectangle dest(0, 0, 50, 50);
        gl::Extents size(50, 50, 1);
        EXPECT_FALSE(rx::ClipBlitRectangles(&source, &dest, size));
    }

    // Only one dimension out of bounds
    {
        gl::Rectangle source(0, 100, 50, 50);
        gl::Rectangle dest(0, 0, 50, 50);
        gl::Extents size(50, 50, 1);
        EXPECT_TRUE(rx::ClipBlitRectangles(&source, &dest, size));
    }
}

}  // namespace angle
