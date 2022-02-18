//
// Copyright 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// MemoryShaderCache: Stores compiled shader in memory so they don't
//   always have to be re-compiled. Can be used in conjunction with the platform
//   layer to warm up the cache from disk.

#include "libANGLE/MemoryShaderCache.h"

#include <GLSLANG/ShaderVars.h>
#include <anglebase/sha1.h>

#include "common/angle_version_info.h"
#include "common/utilities.h"
#include "libANGLE/BinaryStream.h"
#include "libANGLE/Context.h"
#include "libANGLE/Uniform.h"
#include "libANGLE/histogram_macros.h"
#include "libANGLE/renderer/ShaderImpl.h"
#include "platform/PlatformMethods.h"

#include <android/log.h>

namespace gl
{

namespace
{
constexpr unsigned int kWarningLimit = 3;

class HashStream final : angle::NonCopyable
{
  public:
    std::string str() { return mStringStream.str(); }

    template <typename T>
    HashStream &operator<<(T value)
    {
        mStringStream << value << kSeparator;
        return *this;
    }

  private:
    static constexpr char kSeparator = ':';
    std::ostringstream mStringStream;
};

HashStream &operator<<(HashStream &stream, const Shader *shader)
{
    if (shader)
    {
        stream << shader->getType() << shader->getSourceString().length()
               << shader->getSourceString().c_str();
    }
    return stream;
}

}  // anonymous namespace

MemoryShaderCache::MemoryShaderCache(egl::BlobCache &blobCache)
    : mBlobCache(blobCache), mIssuedWarnings(0)
{}

MemoryShaderCache::~MemoryShaderCache() {}

void MemoryShaderCache::ComputeHash(const Context *context,
                                    const Shader *shader,
                                    egl::BlobCache::Key *hashOut)
{
    // Compute the shader hash. Start with the shader hashes and resource strings.
    HashStream hashStream;
    hashStream << shader;

    // Add some ANGLE metadata and Context properties, such as version and back-end.
    hashStream << angle::GetANGLECommitHash() << context->getClientMajorVersion()
               << context->getClientMinorVersion() << context->getString(GL_RENDERER);

    // Call the secure SHA hashing function.
    const std::string &shaderKey = hashStream.str();
    angle::base::SHA1HashBytes(reinterpret_cast<const unsigned char *>(shaderKey.c_str()),
                               shaderKey.length(), hashOut->data());
}

angle::Result MemoryShaderCache::getShader(const Context *context,
                                           Shader *shader,
                                           egl::BlobCache::Key *hashOut)
{
    // If caching is effectively disabled, don't bother calculating the hash.
    if (!mBlobCache.isCachingEnabled())
    {
        return angle::Result::Incomplete;
    }

    ComputeHash(context, shader, hashOut);
    egl::BlobCache::Value binaryShader;
    size_t shaderSize = 0;

    if (get(context, *hashOut, &binaryShader, &shaderSize))
    {
        angle::MemoryBuffer uncompressedData;
        if (!egl::DecompressBlobCacheData(binaryShader.data(), shaderSize, &uncompressedData))
        {
            ERR() << "Error decompressing shader binary data.";
            return angle::Result::Incomplete;
        }

        angle::Result result = shader->loadBinary(context, uncompressedData.data(),
                                                  static_cast<int>(uncompressedData.size()));
        ANGLE_HISTOGRAM_BOOLEAN("GPU.ANGLE.ShaderCache.LoadBinarySuccess",
                                result == angle::Result::Continue);
        ANGLE_TRY(result);

        if (result == angle::Result::Continue)
            return angle::Result::Continue;

        // Cache load failed, evict.
        if (mIssuedWarnings++ < kWarningLimit)
        {
            WARN() << "Failed to load binary from cache.";

            if (mIssuedWarnings == kWarningLimit)
            {
                WARN() << "Reaching warning limit for cache load failures, silencing "
                          "subsequent warnings.";
            }
        }
        remove(*hashOut);
    }

    return angle::Result::Incomplete;
}

bool MemoryShaderCache::get(const Context *context,
                            const egl::BlobCache::Key &shaderHash,
                            egl::BlobCache::Value *shaderOut,
                            size_t *shaderSizeOut)
{
    return mBlobCache.get(context->getScratchBuffer(), shaderHash, shaderOut, shaderSizeOut);
}

bool MemoryShaderCache::getAt(size_t index,
                              const egl::BlobCache::Key **hashOut,
                              egl::BlobCache::Value *shaderOut)
{
    return mBlobCache.getAt(index, hashOut, shaderOut);
}

void MemoryShaderCache::remove(const egl::BlobCache::Key &shaderHash)
{
    mBlobCache.remove(shaderHash);
}

angle::Result MemoryShaderCache::putShader(const egl::BlobCache::Key &shaderHash,
                                           const Shader *shader)
{
    // If caching is effectively disabled, don't bother serializing the shader.
    if (!mBlobCache.isCachingEnabled())
    {
        return angle::Result::Incomplete;
    }

    angle::MemoryBuffer serializedShader;
    ANGLE_TRY(shader->serialize(nullptr, &serializedShader));

    angle::MemoryBuffer compressedData;
    if (!egl::CompressBlobCacheData(serializedShader.size(), serializedShader.data(),
                                    &compressedData))
    {
        ERR() << "Error compressing shader binary data.";
        return angle::Result::Incomplete;
    }

    ANGLE_HISTOGRAM_COUNTS("GPU.ANGLE.ShaderCache.ShaderBinarySizeBytes",
                           static_cast<int>(compressedData.size()));

    mBlobCache.put(shaderHash, std::move(compressedData));
    return angle::Result::Continue;
}

angle::Result MemoryShaderCache::updateShader(const Context *context, const Shader *shader)
{
    egl::BlobCache::Key shaderHash;
    ComputeHash(context, shader, &shaderHash);
    return putShader(shaderHash, shader);
}

bool MemoryShaderCache::putBinary(const egl::BlobCache::Key &shaderHash,
                                  const uint8_t *binary,
                                  size_t length)
{
    // Copy the binary.
    angle::MemoryBuffer newEntry;
    if (!newEntry.resize(length))
    {
        return false;
    }
    memcpy(newEntry.data(), binary, length);

    // Store the binary.
    mBlobCache.populate(shaderHash, std::move(newEntry));

    return true;
}

void MemoryShaderCache::clear()
{
    mBlobCache.clear();
    mIssuedWarnings = 0;
}

void MemoryShaderCache::resize(size_t maxCacheSizeBytes)
{
    mBlobCache.resize(maxCacheSizeBytes);
}

size_t MemoryShaderCache::entryCount() const
{
    return mBlobCache.entryCount();
}

size_t MemoryShaderCache::trim(size_t limit)
{
    return mBlobCache.trim(limit);
}

size_t MemoryShaderCache::size() const
{
    return mBlobCache.size();
}

size_t MemoryShaderCache::maxSize() const
{
    return mBlobCache.maxSize();
}

}  // namespace gl
