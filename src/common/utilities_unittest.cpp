//
// Copyright (c) 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// utilities_unittest.cpp: Unit tests for ANGLE's GL utility functions

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "common/utilities.h"

namespace
{

TEST(ParseResourceName, ArrayIndex)
{
    std::vector<unsigned int> indices;
    EXPECT_EQ("foo", gl::ParseResourceName("foo[123]", &indices));
    ASSERT_EQ(1u, indices.size());
    EXPECT_EQ(123u, indices[0]);

    EXPECT_EQ("bar", gl::ParseResourceName("bar[0]", &indices));
    ASSERT_EQ(1u, indices.size());
    EXPECT_EQ(0u, indices[0]);
}

TEST(ParseResourceName, NegativeArrayIndex)
{
    std::vector<unsigned int> indices;
    EXPECT_EQ("foo", gl::ParseResourceName("foo[-1]", &indices));
    ASSERT_EQ(1u, indices.size());
    EXPECT_EQ(GL_INVALID_INDEX, indices.back());
}

TEST(ParseResourceName, NoArrayIndex)
{
    std::vector<unsigned int> indices;
    EXPECT_EQ("foo", gl::ParseResourceName("foo", &indices));
    EXPECT_EQ(0u, indices.size());
}

TEST(ParseResourceName, NULLArrayIndices)
{
    EXPECT_EQ("foo", gl::ParseResourceName("foo[10]", nullptr));
}

TEST(ParseResourceName, MultipleArrayIndices)
{
    std::vector<unsigned int> indices;
    EXPECT_EQ("foo", gl::ParseResourceName("foo[12][34][56]", &indices));
    ASSERT_EQ(3u, indices.size());
    // Indices are sorted with outermost array index last.
    EXPECT_EQ(56, indices[0]);
    EXPECT_EQ(34, indices[1]);
    EXPECT_EQ(12, indices[2]);
}

TEST(ParseResourceName, TrailingWhitespace)
{
    std::vector<unsigned int> indices;
    EXPECT_EQ("foo ", gl::ParseResourceName("foo ", &indices));
    EXPECT_EQ(0u, indices.size());

    EXPECT_EQ("foo[10] ", gl::ParseResourceName("foo[10] ", &indices));
    EXPECT_EQ(0u, indices.size());

    EXPECT_EQ("foo[10][20] ", gl::ParseResourceName("foo[10][20] ", &indices));
    EXPECT_EQ(0u, indices.size());
}

}
