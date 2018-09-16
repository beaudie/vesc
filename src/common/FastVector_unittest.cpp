//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// FixedVector_unittest:
//   Tests of the FastVector class
//

#include <gtest/gtest.h>

#include "common/FastVector.h"

namespace angle
{
// Make sure the various constructors compile and do basic checks
TEST(FastVector, Constructors)
{
    FastVector<int, 5> defaultContructor;
    EXPECT_EQ(0u, defaultContructor.size());

    FastVector<int, 5> count(3);
    EXPECT_EQ(3u, count.size());

    FastVector<int, 5> countAndValue(3, 2);
    EXPECT_EQ(3u, countAndValue.size());
    EXPECT_EQ(2, countAndValue[1]);

    FastVector<int, 5> copy(countAndValue);
    EXPECT_EQ(copy, countAndValue);

    FastVector<int, 5> copyRValue(std::move(count));
    EXPECT_EQ(3u, copyRValue.size());

    FastVector<int, 5> initializerList{1, 2, 3, 4, 5};
    EXPECT_EQ(5u, initializerList.size());
    EXPECT_EQ(3, initializerList[2]);

    FastVector<int, 5> assignCopy(copyRValue);
    EXPECT_EQ(3u, assignCopy.size());

    FastVector<int, 5> assignRValue(std::move(assignCopy));
    EXPECT_EQ(3u, assignRValue.size());

    FastVector<int, 5> assignmentInitializerList = {1, 2, 3, 4, 5};
    EXPECT_EQ(5u, assignmentInitializerList.size());
    EXPECT_EQ(3, assignmentInitializerList[2]);
}

// Test indexing operations (at, operator[])
TEST(FastVector, Indexing)
{
    FastVector<int, 5> vec = {0, 1, 2, 3, 4};
    EXPECT_EQ(0, vec.at(0));
    EXPECT_EQ(vec[0], vec.at(0));
}

// Test the push_back functions
TEST(FastVector, PushBack)
{
    FastVector<int, 5> vec;
    vec.push_back(1);
    EXPECT_EQ(1, vec[0]);
    vec.push_back(1);
    vec.push_back(1);
    vec.push_back(1);
    vec.push_back(1);
    EXPECT_EQ(5u, vec.size());
}

// Test the pop_back function
TEST(FastVector, PopBack)
{
    FastVector<int, 5> vec;
    vec.push_back(1);
    EXPECT_EQ(1, (int)vec.size());
    vec.pop_back();
    EXPECT_EQ(0, (int)vec.size());
}

// Test the back function
TEST(FastVector, Back)
{
    FastVector<int, 5> vec;
    vec.push_back(1);
    vec.push_back(2);
    EXPECT_EQ(2, vec.back());
}

// Test the sizing operations
TEST(FastVector, Size)
{
    FastVector<int, 5> vec;
    EXPECT_TRUE(vec.empty());
    EXPECT_EQ(0u, vec.size());

    vec.push_back(1);
    EXPECT_FALSE(vec.empty());
    EXPECT_EQ(1u, vec.size());
}

// Test clearing the vector
TEST(FastVector, Clear)
{
    FastVector<int, 5> vec = {0, 1, 2, 3, 4};
    vec.clear();
    EXPECT_TRUE(vec.empty());
}

// Test resizing the vector
TEST(FastVector, Resize)
{
    FastVector<int, 5> vec;
    vec.resize(5u, 1);
    EXPECT_EQ(5u, vec.size());
    EXPECT_EQ(1, vec[4]);

    vec.resize(2u);
    EXPECT_EQ(2u, vec.size());
}

// Test iterating over the vector
TEST(FastVector, Iteration)
{
    FastVector<int, 5> vec = {0, 1, 2, 3};

    int vistedCount = 0;
    for (int value : vec)
    {
        EXPECT_EQ(vistedCount, value);
        vistedCount++;
    }
    EXPECT_EQ(4, vistedCount);
}
}  // namespace angle
