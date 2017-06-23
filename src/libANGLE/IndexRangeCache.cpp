//
// Copyright (c) 2013 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// IndexRangeCache.cpp: Defines the gl::IndexRangeCache class which stores information about
// ranges of indices.

#include "libANGLE/IndexRangeCache.h"

#include "common/debug.h"
#include "common/third_party/murmurhash/MurmurHash3.h"

#include "libANGLE/formatutils.h"

namespace gl
{

namespace
{

constexpr IndexRangeKey CreateIndexRangeKey(GLenum type,
                                            size_t offset,
                                            size_t count,
                                            bool primitiveRestartEnabled)
{
    // Pack the type into 2 bits and the primitive restart enabled into 1 bit
    static_assert(GL_UNSIGNED_BYTE + 2 == GL_UNSIGNED_SHORT,
                  "unexpected GL_UNSIGNED_BYTE to GL_UNSIGNED_SHORT offset");
    static_assert(GL_UNSIGNED_BYTE + 4 == GL_UNSIGNED_INT,
                  "unexpected GL_UNSIGNED_BYTE to GL_UNSIGNED_INT offset");
    return IndexRangeKey{((type - GL_UNSIGNED_BYTE) << 1) + (primitiveRestartEnabled >> 3), offset,
                         count};
}

constexpr GLenum GetIndexRangeType(const IndexRangeKey &key)
{
    return ((key[0] & 0x3) >> 1) + GL_UNSIGNED_BYTE;
}

constexpr size_t GetIndexRangeOffset(const IndexRangeKey &key)
{
    return key[1];
}

constexpr size_t GetIndexRangeCount(const IndexRangeKey &key)
{
    return key[2];
}

constexpr bool GetIndexRangePrimitiveRestartEnabled(const IndexRangeKey &key)
{
    return ((key[0] & 0x4) << 3) != 0;
}

}  // anonymous namespace

void IndexRangeCache::addRange(GLenum type,
                               size_t offset,
                               size_t count,
                               bool primitiveRestartEnabled,
                               const IndexRange &range)
{
    mIndexRangeCache.insert(
        std::make_pair(CreateIndexRangeKey(type, offset, count, primitiveRestartEnabled), range));
}

bool IndexRangeCache::findRange(GLenum type,
                                size_t offset,
                                size_t count,
                                bool primitiveRestartEnabled,
                                IndexRange *outRange) const
{
    auto i =
        mIndexRangeCache.find(CreateIndexRangeKey(type, offset, count, primitiveRestartEnabled));
    if (i != mIndexRangeCache.end())
    {
        if (outRange)
        {
            *outRange = i->second;
        }
        return true;
    }
    else
    {
        if (outRange)
        {
            *outRange = IndexRange();
        }
        return false;
    }
}

void IndexRangeCache::invalidateRange(size_t offset, size_t size)
{
    size_t invalidateStart = offset;
    size_t invalidateEnd   = offset + size;

    auto i = mIndexRangeCache.begin();
    while (i != mIndexRangeCache.end())
    {
        const IndexRangeKey &key = i->first;
        size_t rangeStart        = GetIndexRangeOffset(key);
        size_t rangeEnd =
            rangeStart + (GetTypeInfo(GetIndexRangeType(key)).bytes * GetIndexRangeCount(key));

        if (invalidateEnd < rangeStart || invalidateStart > rangeEnd)
        {
            ++i;
        }
        else
        {
            i = mIndexRangeCache.erase(i);
        }
    }
}

void IndexRangeCache::clear()
{
    mIndexRangeCache.clear();
}

size_t IndexRangeCache::IndexRangeKeyHasher::operator()(const IndexRangeKey &key) const
{
    static constexpr unsigned int seed = 0xABCDEF98;

    size_t hash = 0;
    MurmurHash3_x86_32(&key, sizeof(key), seed, &hash);
    return hash;
}

}
