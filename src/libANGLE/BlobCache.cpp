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
#include "libANGLE/Display.h"
#include "libANGLE/histogram_macros.h"
#include "platform/Platform.h"

namespace gl
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

void BlobCache::put(egl::Display *display, const BlobCache::Key &key, BlobCache::Value &&value)
{
    CacheEntry newEntry;
    newEntry.first = std::move(value);
    // It is assumed that display is always provided, except for during application startup, where
    // they are read from disk.  The `display` pointer is used to test this as during application
    // startup this pointer would be nullptr to avoid generating callbacks.
    // TODO syoussefi: .second to be removed when EGL_ANGLE_program_cache_control support is removed
    newEntry.second = display ? CacheSource::Memory : CacheSource::Disk;

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

bool BlobCache::get(egl::Display *display,
                    const BlobCache::Key &key,
                    const BlobCache::Value **valueOut)
{
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

        *valueOut = &entry->first;
        return true;
    }

    // If the key is not in ANGLE's cache, look into the application's.
    if (display && display->getState().getBlobFunc)
    {
        result = get(key, valueOut, display->getState().getBlobFunc);
    }

    if (!result)
    {
        ANGLE_HISTOGRAM_ENUMERATION("GPU.ANGLE.ProgramCache.CacheResult", kCacheMiss,
                                    kCacheResultMax);
    }

    return result;
}

bool BlobCache::get(const BlobCache::Key &key,
                    const BlobCache::Value **valueOut,
                    EGLGetBlobFuncANDROID getBlobFunc)
{
    EGLsizeiANDROID valueSize = getBlobFunc(key.data(), key.size(), nullptr, 0);
    if (valueSize <= 0)
    {
        return false;
    }

    CacheEntry newEntry;
    newEntry.first.resize(valueSize);
    valueSize = getBlobFunc(key.data(), key.size(), newEntry.first.data(), valueSize);

    if (static_cast<size_t>(valueSize) != newEntry.first.size())
    {
        return false;
    }

    // If succeeded, duplicate the entry in our cache.  This is to avoid the above copy on every
    // get().

    // If the callbacks are set, then put() doesn't add anything to our cache, so the Disk/Memory
    // tag decided there is lost.  However, in that case, we assume the application is handling its
    // own binary info, which means we can only ever get here from angle itself (not through
    // EGL_ANGLE_program_cache_control from the application), so the Memory tag is the right one.
    newEntry.second = CacheSource::Memory;

    const CacheEntry *result = mBlobCache.put(key, std::move(newEntry), newEntry.first.size());
    if (!result)
    {
        ERR() << "Failed to store binary program in memory cache, program is too large.";
        return false;
    }

    *valueOut = &result->first;
    return true;
}

bool BlobCache::getAt(size_t index,
                      const BlobCache::Key **keyOut,
                      const BlobCache::Value **valueOut)
{
    const CacheEntry *valueBuf;
    bool result = mBlobCache.getAt(index, keyOut, &valueBuf);
    if (result)
    {
        *valueOut = &valueBuf->first;
    }
    return result;
}

void BlobCache::remove(const BlobCache::Key &key)
{
    bool result = mBlobCache.eraseByKey(key);
    ASSERT(result);
}

}  // namespace gl
