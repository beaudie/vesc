//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ResourceMap_unittest:
//   Unit tests for the ResourceMap template class.
//

#include <gtest/gtest.h>

#include "libANGLE/ResourceMap.h"

using namespace gl;

namespace
{
TEST(ResourceMapTest, AssignAndErase)
{
    constexpr size_t kSize = 64;
    ResourceMap<size_t> resourceMap;
    std::vector<size_t> objects(kSize, 1);
    for (size_t index = 0; index < kSize; ++index)
    {
        resourceMap.assign(index + 1, &objects[index]);
    }

    for (size_t index = 0; index < kSize; ++index)
    {
        size_t *found = nullptr;
        ASSERT_TRUE(resourceMap.erase(index + 1, &found));
        ASSERT_EQ(&objects[index], found);
    }

    ASSERT_TRUE(resourceMap.empty());
}

TEST(ResourceMapTest, AssignAndClear)
{
    constexpr size_t kSize = 64;
    ResourceMap<size_t> resourceMap;
    std::vector<size_t> objects(kSize, 1);
    for (size_t index = 0; index < kSize; ++index)
    {
        resourceMap.assign(index + 1, &objects[index]);
    }

    resourceMap.clear();
    ASSERT_TRUE(resourceMap.empty());
}
}  // anonymous namespace
