//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// BlobCache: Stores compiled and linked programs in memory so they don't
//   always have to be re-compiled. Can be used in conjunction with the platform
//   layer to warm up the cache from disk.

#ifndef LIBANGLE_BLOB_CACHE_H_
#define LIBANGLE_BLOB_CACHE_H_

#include <array>
#include <cstring>

#include <anglebase/sha1.h>
#include "common/MemoryBuffer.h"
#include "libANGLE/Error.h"
#include "libANGLE/SizedMRUCache.h"

namespace gl
{
class Context;
}  // namespace gl

namespace egl
{
namespace blobcachehelper
{
// 160-bit SHA-1 hash key used for hasing a program.  BlobCache opts in using fixed keys for
// simplicity and efficiency.
static constexpr size_t kKeyLength = angle::base::kSHA1Length;
using Key                          = std::array<uint8_t, kKeyLength>;
}  // namespace blobcachehelper
}  // namespace egl

namespace std
{
template <>
struct hash<egl::blobcachehelper::Key>
{
    // Simple routine to hash four ints.
    size_t operator()(const egl::blobcachehelper::Key &key) const
    {
        return angle::ComputeGenericHash(key.data(), key.size());
    }
};
}  // namespace std

namespace egl
{

class BlobCache final : angle::NonCopyable
{
  public:
    // 160-bit SHA-1 hash key used for hasing a program.  BlobCache opts in using fixed keys for
    // simplicity and efficiency.
    static constexpr size_t kKeyLength = blobcachehelper::kKeyLength;
    using Key                          = blobcachehelper::Key;
    class Value
    {
      public:
        Value() : mPtr(nullptr), mSize(0) {}
        Value(const uint8_t *ptr, size_t sz) : mPtr(ptr), mSize(sz) {}

        // A very basic struct to hold the pointer and size together.  The objects of this class
        // doesn't own the memory.
        const uint8_t *data() { return mPtr; }
        size_t size() { return mSize; }

        const uint8_t &operator[](size_t pos) const
        {
            ASSERT(pos < mSize);
            return mPtr[pos];
        }

      private:
        const uint8_t *mPtr;
        size_t mSize;
    };

    BlobCache(size_t maxCacheSizeBytes);
    ~BlobCache();

    // Store a key-blob pair in the cache.  context can be nullptr, e.g. to repopulate the ANGLE
    // cache on startup without generating callback calls.
    void put(const gl::Context *context, const BlobCache::Key &key, angle::MemoryBuffer &&value);

    // Check if the cache contains the blob corresponding to this key
    bool get(const gl::Context *context, const BlobCache::Key &key, BlobCache::Value *valueOut);

    // For querying the contents of the cache.
    bool getAt(size_t index, const BlobCache::Key **keyOut, BlobCache::Value *valueOut);

    // Evict a blob from the binary cache.
    void remove(const BlobCache::Key &key);

    // Empty the cache.
    void clear() { mBlobCache.clear(); }

    // Resize the cache. Discards current contents.
    void resize(size_t maxCacheSizeBytes) { mBlobCache.resize(maxCacheSizeBytes); }

    // Returns the number of entries in the cache.
    size_t entryCount() const { return mBlobCache.entryCount(); }

    // Reduces the current cache size and returns the number of bytes freed.
    size_t trim(size_t limit) { return mBlobCache.shrinkToSize(limit); }

    // Returns the current cache size in bytes.
    size_t size() const { return mBlobCache.size(); }

    // Returns whether the cache is empty
    bool empty() const { return mBlobCache.empty(); }

    // Returns the maximum cache size in bytes.
    size_t maxSize() const { return mBlobCache.maxSize(); }

  private:
    enum class CacheSource
    {
        Memory,
        Disk,
    };

    // This internal cache is used only if the application is not providing caching callbacks
    using CacheEntry = std::pair<angle::MemoryBuffer, CacheSource>;
    angle::SizedMRUCache<BlobCache::Key, CacheEntry> mBlobCache;
};

}  // namespace egl

#endif  // LIBANGLE_MEMORY_PROGRAM_CACHE_H_
