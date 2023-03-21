//
// Copyright 2023 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// mtl_library_cache.mm:
//    Defines classes for caching of mtl libraries
//

#include "libANGLE/renderer/metal/mtl_library_cache.h"

#include <stdio.h>

#include <limits>

#include "common/MemoryBuffer.h"
#include "common/angleutils.h"
#include "common/hash_utils.h"
#include "common/mathutil.h"
#include "common/string_utils.h"
#include "common/system_utils.h"
#include "libANGLE/renderer/metal/ContextMtl.h"
#include "libANGLE/renderer/metal/mtl_context_device.h"
#include "libANGLE/renderer/metal/process.h"

namespace rx
{
namespace mtl
{

AutoObjCPtr<id<MTLLibrary>> LibraryCache::get(const std::string &source,
                                              const std::map<std::string, std::string> &macros,
                                              bool enableFastMath)
{
    LibraryKey::LValueTuple key            = std::tie(source, macros, enableFastMath);
    LibraryCache::LibraryCacheEntry &entry = getCacheEntry(key);

    // Try to lock the entry and return the library if it exists. If we can't lock then it means
    // another thread is currently compiling.
    std::unique_lock<std::mutex> entryLockGuard(entry.lock, std::try_to_lock);
    if (entryLockGuard)
    {
        return entry.library;
    }
    else
    {
        return nil;
    }
}

namespace
{

// Reads a metallib file at the specified path.
angle::MemoryBuffer ReadMetallibFromFile(const std::string &path)
{
    // TODO: optimize this to avoid the unnecessary strings.
    std::string metallib;
    if (!angle::ReadFileToString(path, &metallib))
    {
        FATAL() << "Failed reading back metallib";
    }

    angle::MemoryBuffer buffer;
    if (!buffer.resize(metallib.size()))
    {
        FATAL() << "Failed to resize metallib buffer";
    }
    memcpy(buffer.data(), metallib.data(), metallib.size());
    return buffer;
}

// Generates a key for the BlobCache based on the specified params.
egl::BlobCacheKey GenerateBlobCacheKeyForShaderLibrary(
    const std::string &source,
    const std::map<std::string, std::string> &macros,
    bool enableFastMath)
{
    angle::base::SecureHashAlgorithm sha1;
    sha1.Update(source.c_str(), source.size());
    const size_t macro_count = macros.size();
    sha1.Update(&macro_count, sizeof(size_t));
    for (const auto &macro : macros)
    {
        sha1.Update(macro.first.c_str(), macro.first.size());
        sha1.Update(macro.second.c_str(), macro.second.size());
    }
    sha1.Update(&enableFastMath, sizeof(bool));
    sha1.Final();
    return sha1.DigestAsArray();
}

std::string BlobCacheKeyToString(const egl::BlobCacheKey &key)
{
  const char kUpCaseHexDigits[]   = "0123456789ABCDEF";
    std::basic_ostringstream<char> stream;
    for (size_t i = 0; i < key.size(); ++i)
    {
      int value = (int)key[i];
      stream << kUpCaseHexDigits[value / 16];
      stream << kUpCaseHexDigits[value % 16];
        stream << "-";
    }
    return stream.str();
}

void AppendMetallibToFile(const egl::BlobCacheKey &cache_key, const angle::MemoryBuffer &buffer)
{
    static FILE* file = nullptr;
    if (!file)
    {
        file = fopen("/tmp/shaders", "w");
        if (!file)
        {
            FATAL() << "Unable to open file to save shaders to";
        }
    }
    static int writeCount = 0;
    if (buffer.size() > std::numeric_limits<uint32_t>::max() ||
        cache_key.size() > std::numeric_limits<uint32_t>::max()) {
      FATAL() << "Shader key and/or buffer size is too large";
    }
    printf("writing shader key %s\n", BlobCacheKeyToString(cache_key).c_str());
    const uint32_t key_length   = static_cast<uint32_t>(cache_key.size());
    const uint32_t value_length = static_cast<uint32_t>(buffer.size());
    if (fwrite(&key_length, sizeof(key_length), 1, file) != 1 ||
        fwrite(cache_key.data(), sizeof(egl::BlobCacheKey::value_type), cache_key.size(), file) !=
            cache_key.size() ||
        fwrite(&value_length, sizeof(value_length), 1, file) != 1 ||
        fwrite(buffer.data(), sizeof(char), buffer.size(), file) != buffer.size())
    {
        FATAL() << "Failed writing shader to file";
    }
    // Flush as we won't have a change to close the file.
    fflush(file);
    printf("Wrote another metallib %d\n", ++writeCount);
}

// Returns a new MTLLibrary from the specified data.
AutoObjCPtr<id<MTLLibrary>> NewMetalLibraryFromMetallib(ContextMtl *context,
                                                        const uint8_t *data,
                                                        size_t size)
{
    ANGLE_MTL_OBJC_SCOPE
    {
        // DISPATCH_DATA_DESTRUCTOR_DEFAULT makes a copy. I'm assuming we want that
        // as the life of the BlobCache is not necessarily the same as that of the
        // metallibrary.
        auto mtl_data = dispatch_data_create(data, size, dispatch_get_main_queue(),
                                             DISPATCH_DATA_DESTRUCTOR_DEFAULT);

        NSError *nsError = nil;
        return context->getMetalDevice().newLibraryWithData(mtl_data, &nsError);
    }
}

}  // namespace

AutoObjCPtr<id<MTLLibrary>> LibraryCache::getOrCompileShaderLibrary(
    ContextMtl *context,
    const std::string &source,
    const std::map<std::string, std::string> &macros,
    bool enableFastMath,
    AutoObjCPtr<NSError *> *errorOut)
{
    const angle::FeaturesMtl &features = context->getDisplay()->getFeatures();
    if (!features.enableInMemoryMtlLibraryCache.enabled)
    {
        return CreateShaderLibrary(context->getMetalDevice(), source, macros, enableFastMath,
                                   errorOut);
    }

    LibraryKey::LValueTuple key            = std::tie(source, macros, enableFastMath);
    LibraryCache::LibraryCacheEntry &entry = getCacheEntry(key);

    // Lock this cache entry while compiling the shader. This causes other threads calling this
    // function to wait and not duplicate the compilation.
    std::lock_guard<std::mutex> entryLockGuard(entry.lock);
    if (entry.library)
    {
        return entry.library;
    }

    if (features.compileMetalShaders.enabled)
    {
        if (features.enableParallelMtlLibraryCompilation.enabled)
        {
            // When enableParallelMtlLibraryCompilation is enabled, compilation happens in the
            // background. Chrome's ProgramCache only saves to disk when called at certain points,
            // which are not present when compiling in the background.
            FATAL() << "EnableParallelMtlLibraryCompilation is not compatible with "
                       "compileMetalShdaders";
        }
        std::string metallib_filename = CompileShaderLibraryToFile(source, macros, enableFastMath);
        angle::MemoryBuffer memory_buffer = ReadMetallibFromFile(metallib_filename);
        entry.library =
            NewMetalLibraryFromMetallib(context, memory_buffer.data(), memory_buffer.size());
        auto cache_key = GenerateBlobCacheKeyForShaderLibrary(source, macros, enableFastMath);
        if (features.generateMetalShadersFile.enabled)
        {
            printf("XXX adding to shader cache %s\n", BlobCacheKeyToString(cache_key).c_str());
            AppendMetallibToFile(cache_key, memory_buffer);
        }
        else
        {
            printf("XXX adding to cache %s\n", BlobCacheKeyToString(cache_key).c_str());
            context->getDisplay()->getBlobCache()->put(cache_key, std::move(memory_buffer));
            printf("XXX added\n");
        }
        return entry.library;
    }

    if (features.loadMetalShadersFromBlobCache.enabled)
    {
        WARN() << "Trying to load metallib from BlobCache";
        auto cache_key = GenerateBlobCacheKeyForShaderLibrary(source, macros, enableFastMath);
        egl::BlobCache::Value value;
        size_t buffer_size;
        angle::ScratchBuffer scratch_buffer;
        if (context->getDisplay()->getBlobCache()->get(&scratch_buffer, cache_key, &value,
                                                       &buffer_size))
        {
            entry.library = NewMetalLibraryFromMetallib(context, value.data(), value.size());
            if (entry.library)
            {
                WARN() << "Found metallib in BlobCache";
                printf("XXX found key in cache %s\n", BlobCacheKeyToString(cache_key).c_str());
                return entry.library;
            }
        }
    }

    entry.library =
        CreateShaderLibrary(context->getMetalDevice(), source, macros, enableFastMath, errorOut);
    return entry.library;
}

LibraryCache::LibraryCacheEntry &LibraryCache::getCacheEntry(
    const LibraryKey::LValueTuple &lValueKey)
{
    // Lock while searching or adding new items to the cache.
    std::lock_guard<std::mutex> cacheLockGuard(mCacheLock);

#if ANGLE_HAS_HASH_MAP_GENERIC_LOOKUP
    // Fast-path that can search the cache with only lvalues instead of making a copy of the key
    auto iter = mCache.find(lValueKey);
    if (iter != mCache.end())
    {
        return iter->second;
    }
#endif

    LibraryKey key(lValueKey);
    return mCache[std::move(key)];
}

LibraryCache::LibraryKey::LibraryKey(const LValueTuple &fromTuple)
{
    source         = std::get<0>(fromTuple);
    macros         = std::get<1>(fromTuple);
    enableFastMath = std::get<2>(fromTuple);
}

LibraryCache::LibraryKey::LValueTuple LibraryCache::LibraryKey::tie() const
{
    return std::tie(source, macros, enableFastMath);
}

size_t LibraryCache::LibraryKeyCompare::operator()(const LibraryKey::LValueTuple &k) const
{
    size_t hash = std::hash<std::string>()(std::get<0>(k));
    for (const auto &macro : std::get<1>(k))
    {
        hash =
            hash ^ std::hash<std::string>()(macro.first) ^ std::hash<std::string>()(macro.second);
    }
    hash = hash ^ std::hash<bool>()(std::get<2>(k));
    return hash;
}

size_t LibraryCache::LibraryKeyCompare::operator()(const LibraryKey &k) const
{
    return operator()(k.tie());
}

bool LibraryCache::LibraryKeyCompare::operator()(const LibraryKey &a, const LibraryKey &b) const
{
    return a.tie() == b.tie();
}

bool LibraryCache::LibraryKeyCompare::operator()(const LibraryKey &a,
                                                 const LibraryKey::LValueTuple &b) const
{
    return a.tie() == b;
}

bool LibraryCache::LibraryKeyCompare::operator()(const LibraryKey::LValueTuple &a,
                                                 const LibraryKey &b) const
{
    return a == b.tie();
}

}  // namespace mtl
}  // namespace rx
