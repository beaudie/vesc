//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// hash_utils_unittest: Hashing helper functions tests.

#include <gtest/gtest.h>

#include "common/hash_utils.h"

using namespace angle;

namespace
{
// Basic functionality test.
TEST(HashUtilsTest, ComputeGenericHash)
{
    std::string a = "aSimpleString";
    std::string b = "anotherString";

    size_t aHash = ComputeGenericHash(a.c_str(), a.size());
    size_t bHash = ComputeGenericHash(b.c_str(), b.size());

    EXPECT_NE(aHash, bHash);
}
}  // anonymous namespace
