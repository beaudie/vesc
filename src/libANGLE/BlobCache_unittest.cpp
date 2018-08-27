//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// BlobCache_unittest.h: Unit tests for the blob cache.

#include <gtest/gtest.h>
#include "libANGLE/BlobCache.h"

namespace gl
{

// Note: this is fairly similar to SizedMRUCache_unittest, and makes sure the
// BlobCache usage of SizedMRUCache is not broken.

using Blob = std::vector<uint8_t>;

Blob MakeBlob(size_t size, uint8_t start = 0)
{
    Blob blob;
    for (uint8_t value = 0; value < size; ++value)
    {
        blob.push_back(value + start);
    }
    return blob;
}

Blob MakeKey(size_t size, uint8_t start = 0)
{
    return MakeBlob(size, start);
}

// Test a cache with a value that takes up maximum size.
TEST(BlobCacheTest, MaxSizedValue)
{
    constexpr size_t kSize = 32;
    BlobCache blobCache(kSize);

    blobCache.put(nullptr, MakeKey(4).data(), 4, MakeBlob(kSize).data(), kSize);
    EXPECT_EQ(32u, blobCache.size());
    EXPECT_FALSE(blobCache.empty());

    blobCache.put(nullptr, MakeKey(5).data(), 5, MakeBlob(kSize).data(), kSize);
    EXPECT_EQ(32u, blobCache.size());
    EXPECT_FALSE(blobCache.empty());

    Blob blob;
    EXPECT_FALSE(blobCache.get(nullptr, MakeKey(4).data(), 4, &blob));

    blobCache.clear();
    EXPECT_TRUE(blobCache.empty());
}

// Test a cache with many small values, that it can handle unlimited inserts.
TEST(BlobCacheTest, ManySmallValues)
{
    constexpr size_t kSize = 32;
    BlobCache blobCache(kSize);

    for (size_t value = 0; value < kSize; ++value)
    {
        blobCache.put(nullptr, MakeKey(1, value).data(), 1, MakeBlob(1, value).data(), 1);

        Blob qvalue;
        EXPECT_TRUE(blobCache.get(nullptr, MakeKey(1, value).data(), 1, &qvalue));
        if (!qvalue.empty())
        {
            EXPECT_EQ(value, qvalue[0]);
        }
    }

    EXPECT_EQ(32u, blobCache.size());
    EXPECT_FALSE(blobCache.empty());

    // Putting one element evicts the first element.
    blobCache.put(nullptr, MakeKey(1, kSize).data(), 1, MakeBlob(1, kSize).data(), 1);

    Blob qvalue;
    EXPECT_FALSE(blobCache.get(nullptr, MakeKey(1, 0).data(), 1, &qvalue));

    // Putting one large element cleans out the whole stack.
    blobCache.put(nullptr, MakeKey(1, kSize + 1).data(), 1, MakeBlob(kSize, kSize + 1).data(), kSize);
    EXPECT_EQ(32u, blobCache.size());
    EXPECT_FALSE(blobCache.empty());

    for (size_t value = 0; value <= kSize; ++value)
    {
        EXPECT_FALSE(blobCache.get(nullptr, MakeKey(1, value).data(), 1, &qvalue));
    }
    EXPECT_TRUE(blobCache.get(nullptr, MakeKey(1, kSize + 1).data(), 1, &qvalue));
    if (!qvalue.empty())
    {
        EXPECT_EQ(kSize + 1, qvalue[0]);
    }

    // Put a bunch of items in the cache sequentially.
    for (size_t value = 0; value < kSize * 10; ++value)
    {
        blobCache.put(nullptr, MakeKey(1, value).data(), 1, MakeBlob(1, value).data(), 1);
    }

    EXPECT_EQ(32u, blobCache.size());
}

// Tests putting an oversize element.
TEST(BlobCacheTest, OversizeValue)
{
    constexpr size_t kSize = 32;
    BlobCache blobCache(kSize);

    blobCache.put(nullptr, MakeKey(1, 5).data(), 1, MakeKey(100, 5).data(), 100);

    Blob qvalue;
    EXPECT_FALSE(blobCache.get(nullptr, MakeKey(1, 5).data(), 1, &qvalue));
}

}  // namespace angle
