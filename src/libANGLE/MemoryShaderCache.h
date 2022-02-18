//
// Copyright 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// MemoryShaderCache: Stores compiled shaders in memory so they don't
//   always have to be re-compiled. Can be used in conjunction with the platform
//   layer to warm up the cache from disk.

#ifndef LIBANGLE_MEMORY_SHADER_CACHE_H_
#define LIBANGLE_MEMORY_SHADER_CACHE_H_

#include <array>

#include "common/MemoryBuffer.h"
#include "libANGLE/BlobCache.h"
#include "libANGLE/Error.h"

namespace gl
{
class Context;
class Shader;
class ShaderState;

class MemoryShaderCache final : angle::NonCopyable
{
  public:
    explicit MemoryShaderCache(egl::BlobCache &blobCache);
    ~MemoryShaderCache();

    static void ComputeHash(const Context *context,
                            const Shader *shader,
                            egl::BlobCache::Key *hashOut);

    // Check if the cache contains a binary matching the specified shader.
    bool get(const Context *context,
             const egl::BlobCache::Key &shaderHash,
             egl::BlobCache::Value *shaderOut,
             size_t *shaderSizeOut);

    // For querying the contents of the cache.
    bool getAt(size_t index, const egl::BlobCache::Key **hashOut, egl::BlobCache::Value *shaderOut);

    // Evict a shader from the binary cache.
    void remove(const egl::BlobCache::Key &shaderHash);

    // Helper method that serializes a shader.
    angle::Result putShader(const egl::BlobCache::Key &shaderHash, const Shader *shader);

    // Same as putShader but computes the hash.
    angle::Result updateShader(const Context *context, const Shader *shader);

    // Store a binary directly.  TODO(syoussefi): deprecated.  Will be removed once Chrome supports
    // EGL_ANDROID_blob_cache. http://anglebug.com/2516
    ANGLE_NO_DISCARD bool putBinary(const egl::BlobCache::Key &shaderHash,
                                    const uint8_t *binary,
                                    size_t length);

    // Check the cache, and deserialize and load the shader if found. Evict existing hash if load
    // fails.
    angle::Result getShader(const Context *context, Shader *shader, egl::BlobCache::Key *hashOut);

    // Empty the cache.
    void clear();

    // Resize the cache. Discards current contents.
    void resize(size_t maxCacheSizeBytes);

    // Returns the number of entries in the cache.
    size_t entryCount() const;

    // Reduces the current cache size and returns the number of bytes freed.
    size_t trim(size_t limit);

    // Returns the current cache size in bytes.
    size_t size() const;

    // Returns the maximum cache size in bytes.
    size_t maxSize() const;

    std::mutex &getMutex() { return mMutex; }

  private:
    egl::BlobCache &mBlobCache;
    unsigned int mIssuedWarnings;
    std::mutex mMutex;
};

}  // namespace gl

#endif  // LIBANGLE_MEMORY_SHADER_CACHE_H_
