//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// BlobCache: Stores keyed blobs in memory to support EGL_ANDROID_blob_cache.
// Can be used in conjunction with the platform layer to warm up the cache from
// disk.  MemoryProgramCache uses this to handle caching of compiled programs.

#include "libANGLE/BlobCache.h"
#include "common/utilities.h"
#include "common/version.h"
#include "libANGLE/Context.h"
#include "libANGLE/Display.h"
#include "libANGLE/histogram_macros.h"
#include "platform/Platform.h"

namespace egl
{

namespace
{
enum CacheResult
{
    kCacheMiss,
    kCacheHitMemory,
    kCacheHitDisk,
    kCacheResultMax,
};

}  // anonymous namespace

BlobCache::BlobCache(size_t maxCacheSizeBytes) : mBlobCache(maxCacheSizeBytes)
{
}

BlobCache::~BlobCache()
{
}

void BlobCache::put(const gl::Context *context,
                    const BlobCache::Key &key,
                    angle::MemoryBuffer &&value)
{
    CacheEntry newEntry;
    newEntry.first = std::move(value);
    // It is assumed that context is always provided, except for during application startup, where
    // they are read from disk.  The `context` pointer is used to test this as during application
    // startup this pointer would be nullptr to avoid generating callbacks.
    // TODO(syoussefi): .second to be removed when EGL_ANGLE_program_cache_control support is
    // removed. http://anglebug.com/2516
    newEntry.second = context ? CacheSource::Memory : CacheSource::Disk;

    Display *display = context ? context->getCurrentDisplay() : nullptr;
    if (display && display->getState().setBlobFunc)
    {
        // Store the result in the application's cache
        display->getState().setBlobFunc(key.data(), key.size(), newEntry.first.data(),
                                        newEntry.first.size());
    }
    else
    {
        // Cache it inside blob cache only if caching inside the application is not possible.
        const CacheEntry *result = mBlobCache.put(key, std::move(newEntry), newEntry.first.size());
        if (!result)
        {
            ERR() << "Failed to store binary program in memory cache, program is too large.";
        }
    }
}

bool BlobCache::get(const gl::Context *context,
                    const BlobCache::Key &key,
                    BlobCache::Value *valueOut)
{
    // Look into the application's cache, if there is such a cache
    Display *display = context ? context->getCurrentDisplay() : nullptr;
    if (display && display->getState().getBlobFunc)
    {
        EGLsizeiANDROID valueSize =
            display->getState().getBlobFunc(key.data(), key.size(), nullptr, 0);
        if (valueSize <= 0)
        {
            return false;
        }

        angle::MemoryBuffer *scratchBuffer;
        bool result = context->getScratchBuffer(valueSize, &scratchBuffer);
        if (!result)
        {
            return false;
        }

        valueSize = display->getState().getBlobFunc(key.data(), key.size(), scratchBuffer->data(),
                                                    valueSize);

        // Make sure the key/value pair still exists/is unchagned after the second call
        // (modifications to the application cache by another thread are a possibility)
        if (static_cast<size_t>(valueSize) != scratchBuffer->size())
        {
            return false;
        }

        *valueOut = BlobCache::Value(scratchBuffer->data(), scratchBuffer->size());
        return true;
    }

    // Otherwise we are doing caching internally, so try to find it there
    const CacheEntry *entry;
    bool result = mBlobCache.get(key, &entry);

    if (result)
    {
        if (entry->second == CacheSource::Memory)
        {
            ANGLE_HISTOGRAM_ENUMERATION("GPU.ANGLE.ProgramCache.CacheResult", kCacheHitMemory,
                                        kCacheResultMax);
        }
        else
        {
            ANGLE_HISTOGRAM_ENUMERATION("GPU.ANGLE.ProgramCache.CacheResult", kCacheHitDisk,
                                        kCacheResultMax);
        }

        *valueOut = BlobCache::Value(entry->first.data(), entry->first.size());
    }
    else
    {
        ANGLE_HISTOGRAM_ENUMERATION("GPU.ANGLE.ProgramCache.CacheResult", kCacheMiss,
                                    kCacheResultMax);
    }

    return result;
}

bool BlobCache::getAt(size_t index, const BlobCache::Key **keyOut, BlobCache::Value *valueOut)
{
    const CacheEntry *valueBuf;
    bool result = mBlobCache.getAt(index, keyOut, &valueBuf);
    if (result)
    {
        *valueOut = BlobCache::Value(valueBuf->first.data(), valueBuf->first.size());
    }
    return result;
}

void BlobCache::remove(const BlobCache::Key &key)
{
    bool result = mBlobCache.eraseByKey(key);
    ASSERT(result);
}

}  // namespace gl
