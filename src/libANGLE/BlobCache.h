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

#include "libANGLE/Error.h"
#include "libANGLE/SizedMRUCache.h"

namespace std
{
template <>
struct hash<std::vector<uint8_t>>
{
    // Simple routine to hash four ints.
    size_t operator()(const std::vector<uint8_t> &mem) const
    {
        return angle::ComputeGenericHash(mem.data(), mem.size());
    }
};
}  // namespace std

namespace egl
{
class Display;
}

namespace gl
{
class BlobCache final : angle::NonCopyable
{
  public:
    BlobCache(size_t maxCacheSizeBytes);
    ~BlobCache();

    // Store a key-blob pair in the cache.  display can be nullptr, e.g. to repopulate the ANGLE cache on startup
    // without generating callback calls.
    void put(egl::Display *display, const void *key, size_t keySize, const void *value, size_t valueSize);

    // Check if the cache contains the blob corresponding to this key
    bool get(egl::Display *display, const void *key, size_t keySize, std::vector<uint8_t> *valueOut);

    // For querying the contents of the cache.
    bool getAt(size_t index, const std::vector<uint8_t> **keyOut, const std::vector<uint8_t> **valueOut);

    // Evict a blob from the binary cache.
    void remove(const void *key, size_t keySize);

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

    static void makeKey(const void *key, size_t keySize, std::vector<uint8_t> *keyBuf)
    {
        // See comment on ComputeGenericHash()
        size_t paddedSize = (keySize + 3) & ~(size_t)0x3;

        keyBuf->resize(paddedSize);
        std::memcpy(keyBuf->data(), key, keySize);
        std::memset(keyBuf->data() + keySize, 0, paddedSize - keySize);
    }

  private:
    enum class CacheSource
    {
        Memory,
        Disk,
    };

    using CacheEntry = std::pair<std::vector<uint8_t>, CacheSource>;
    angle::SizedMRUCache<std::vector<uint8_t>, CacheEntry> mBlobCache;
};

}  // namespace gl

#endif  // LIBANGLE_MEMORY_PROGRAM_CACHE_H_
