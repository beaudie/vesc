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

void BlobCache::put(egl::Display *display,
                    const void *key,
                    size_t keySize,
                    const void *value,
                    size_t valueSize)
{
    std::vector<uint8_t> keyBuf;
    makeKey(key, keySize, &keyBuf);

    CacheEntry valueBuf;
    valueBuf.first.resize(valueSize);
    std::memcpy(valueBuf.first.data(), value, valueSize);
    // It is assumed that display is always provided, except for during application startup, where
    // they are read from disk.  The `display` pointer is used to test this as during application
    // startup this pointer would be nullptr to avoid generating callbacks.
    valueBuf.second = display ? CacheSource::Memory : CacheSource::Disk;

    const CacheEntry *result = mBlobCache.put(keyBuf, std::move(valueBuf), valueBuf.first.size());
    if (!result)
    {
        ERR() << "Failed to store binary program in memory cache, program is too large.";
    }
    else if (display)
    {
        // Store the result in the application's cache as well, for potential disk backing
        display->getState().setBlobFunc(key, keySize, value, valueSize);
    }
}

bool BlobCache::get(egl::Display *display,
                    const void *key,
                    size_t keySize,
                    std::vector<uint8_t> *valueOut)
{
    std::vector<uint8_t> keyBuf;
    makeKey(key, keySize, &keyBuf);

    const CacheEntry *valueBuf;
    bool result = mBlobCache.get(keyBuf, &valueBuf);

    if (!result)
    {
        // TODO syoussefi: Change from ProgramCache to BlobCache
        ANGLE_HISTOGRAM_ENUMERATION("GPU.ANGLE.ProgramCache.CacheResult", kCacheMiss,
                                    kCacheResultMax);
    }
    else if (valueBuf->second == CacheSource::Memory)
    {
        ANGLE_HISTOGRAM_ENUMERATION("GPU.ANGLE.ProgramCache.CacheResult", kCacheHitMemory,
                                    kCacheResultMax);
    }
    else
    {
        ANGLE_HISTOGRAM_ENUMERATION("GPU.ANGLE.ProgramCache.CacheResult", kCacheHitDisk,
                                    kCacheResultMax);
    }

    if (result)
    {
        *valueOut = valueBuf->first;
    }
    else if (display)
    {
        // If the key is not in ANGLE's cache, look into the application's.
        EGLsizeiANDROID valueSize = display->getState().getBlobFunc(key, keySize, nullptr, 0);
        if (valueSize > 0)
        {
            valueOut->resize(valueSize);
            display->getState().getBlobFunc(key, keySize, valueOut->data(), valueSize);
            result = true;
        }
    }

    return result;
}

bool BlobCache::getAt(size_t index,
                      const std::vector<uint8_t> **keyOut,
                      const std::vector<uint8_t> **valueOut)
{
    const CacheEntry *valueBuf;
    bool result = mBlobCache.getAt(index, keyOut, &valueBuf);
    if (result)
    {
        *valueOut = &valueBuf->first;
    }
    return result;
}

void BlobCache::remove(const void *key, size_t keySize)
{
    std::vector<uint8_t> keyBuf;
    makeKey(key, keySize, &keyBuf);

    bool result = mBlobCache.eraseByKey(keyBuf);
    ASSERT(result);
}

}  // namespace gl
