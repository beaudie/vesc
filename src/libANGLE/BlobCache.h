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

namespace egl
{
class Display;
}

namespace gl
{
namespace blobcachehelper
{
// 160-bit SHA-1 hash key used for hasing a program.  BlobCache opts in using fixed keys for
// simplicity and efficiency.
static constexpr size_t kKeyLength = angle::base::kSHA1Length;
using Key                          = std::array<uint8_t, kKeyLength>;
using Value                        = angle::MemoryBuffer;
}  // namespace blobcachehelper
}  // namespace gl

namespace std
{
template <>
struct hash<gl::blobcachehelper::Key>
{
    // Simple routine to hash four ints.
    size_t operator()(const gl::blobcachehelper::Key &key) const
    {
        unsigned int hash = 0;
        for (uint32_t num : key)
        {
            hash *= 37;
            hash += num;
        }
        return hash;
    }
};
}  // namespace std

namespace gl
{

class BlobCache final : angle::NonCopyable
{
  public:
    // 160-bit SHA-1 hash key used for hasing a program.  BlobCache opts in using fixed keys for
    // simplicity and efficiency.
    static constexpr size_t kKeyLength = blobcachehelper::kKeyLength;
    using Key                          = blobcachehelper::Key;
    using Value                        = blobcachehelper::Value;

    BlobCache(size_t maxCacheSizeBytes);
    ~BlobCache();

    // Store a key-blob pair in the cache.  display can be nullptr, e.g. to repopulate the ANGLE
    // cache on startup without generating callback calls.
    void put(egl::Display *display, const BlobCache::Key &key, BlobCache::Value &&value);

    // Check if the cache contains the blob corresponding to this key
    bool get(egl::Display *display, const BlobCache::Key &key, const BlobCache::Value **valueOut);

    // For querying the contents of the cache.
    bool getAt(size_t index, const BlobCache::Key **keyOut, const BlobCache::Value **valueOut);

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

    using CacheEntry = std::pair<BlobCache::Value, CacheSource>;
    angle::SizedMRUCache<BlobCache::Key, CacheEntry> mBlobCache;

    // Helper function for retrieving a cache entry from the application
    bool get(const BlobCache::Key &key,
             const BlobCache::Value **valueOut,
             EGLGetBlobFuncANDROID getBlobFunc);
};

}  // namespace gl

#endif  // LIBANGLE_MEMORY_PROGRAM_CACHE_H_
