//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// fixed_vector_unittest:
//   Tests of the fixed_vector class
//

#include <gtest/gtest.h>

#include "common/FixedVector.h"

namespace angle
{
// Make sure the various constructors compile and do basic checks
TEST(FixedVector, Constructors)
{
    FixedVector<int, 5> default_contructor;
    EXPECT_EQ(0u, default_contructor.size());

    FixedVector<int, 5> count(3);
    EXPECT_EQ(3u, count.size());

    FixedVector<int, 5> count_and_value(3, 2);
    EXPECT_EQ(3u, count_and_value.size());
    EXPECT_EQ(2, count_and_value[1]);

    FixedVector<int, 5> copy(count_and_value);
    EXPECT_EQ(copy, count_and_value);

    FixedVector<int, 5> copy_rvalue(count);
    EXPECT_EQ(3u, copy_rvalue.size());

    FixedVector<int, 5> initializer_list{1, 2, 3, 4, 5};
    EXPECT_EQ(5u, initializer_list.size());
    EXPECT_EQ(3, initializer_list[2]);

    FixedVector<int, 5> assign_copy(copy_rvalue);
    EXPECT_EQ(3u, assign_copy.size());

    FixedVector<int, 5> assign_rvalue(assign_copy);
    EXPECT_EQ(3u, assign_rvalue.size());

    FixedVector<int, 5> assignment_initializer_list = {1, 2, 3, 4, 5};
    EXPECT_EQ(5u, assignment_initializer_list.size());
    EXPECT_EQ(3, assignment_initializer_list[2]);
}

// Test indexing operations (at, operator[])
TEST(FixedVector, Indexing)
{
    FixedVector<int, 5> vec = {0, 1, 2, 3, 4};
    EXPECT_EQ(0, vec.at(0));
    EXPECT_EQ(vec[0], vec.at(0));
}

// Test the push_back functions
TEST(FixedVector, PushBack)
{
    FixedVector<int, 5> vec;
    vec.push_back(1);
    EXPECT_EQ(1, vec[0]);
}

// Test the sizing operations
TEST(FixedVector, Size)
{
    FixedVector<int, 5> vec;
    EXPECT_TRUE(vec.empty());
    EXPECT_EQ(0u, vec.size());
    EXPECT_EQ(5u, vec.max_size());

    vec.push_back(1);
    EXPECT_FALSE(vec.empty());
    EXPECT_EQ(1u, vec.size());
}

// Test clearing the vector
TEST(FixedVector, Clear)
{
    FixedVector<int, 5> vec = {0, 1, 2, 3, 4};
    vec.clear();
    EXPECT_TRUE(vec.empty());
}

// Test resizing the vector
TEST(FixedVector, Resize)
{
    FixedVector<int, 5> vec;
    vec.resize(5u, 1);
    EXPECT_EQ(5u, vec.size());
    EXPECT_EQ(1, vec[4]);

    vec.resize(2u);
    EXPECT_EQ(2u, vec.size());
}

// Test iterating over the vector
TEST(FixedVector, Iteration)
{
    FixedVector<int, 5> vec = { 0, 1, 2, 3 };

    int vistedCount = 0;
    for (int value : vec)
    {
        EXPECT_EQ(vistedCount, value);
        vistedCount++;
    }
    EXPECT_EQ(4, vistedCount);
}

} // namespace angle
