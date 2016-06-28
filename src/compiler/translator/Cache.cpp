//
// Copyright (c) 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// Cache.cpp: Implements a cache for various commonly created objects.

#include <limits>

#include "common/angleutils.h"
#include "common/debug.h"
#include "common/tls.h"
#include "compiler/translator/Cache.h"

namespace
{

class TScopedAllocator : angle::NonCopyable
{
  public:
    TScopedAllocator(TPoolAllocator *allocator)
        : mPreviousAllocator(GetGlobalPoolAllocator())
    {
        SetGlobalPoolAllocator(allocator);
    }
    ~TScopedAllocator()
    {
        SetGlobalPoolAllocator(mPreviousAllocator);
    }

  private:
    TPoolAllocator *mPreviousAllocator;
};

}  // namespace

TCache::TypeKey::TypeKey(TBasicType basicType,
                         TPrecision precision,
                         TQualifier qualifier,
                         unsigned char primarySize,
                         unsigned char secondarySize)
{
    static_assert(sizeof(components) <= sizeof(value),
                  "TypeKey::value is too small");

    const size_t MaxEnumValue = std::numeric_limits<EnumComponentType>::max();
    UNUSED_ASSERTION_VARIABLE(MaxEnumValue);

    // TODO: change to static_assert() once we deprecate MSVC 2013 support
    ASSERT(MaxEnumValue >= EbtLast &&
           MaxEnumValue >= EbpLast &&
           MaxEnumValue >= EvqLast &&
           "TypeKey::EnumComponentType is too small");

    value = 0;
    components.basicType = static_cast<EnumComponentType>(basicType);
    components.precision = static_cast<EnumComponentType>(precision);
    components.qualifier = static_cast<EnumComponentType>(qualifier);
    components.primarySize = primarySize;
    components.secondarySize = secondarySize;
}

TLSIndex CacheIndex = TLS_INVALID_INDEX;

bool InitializeTCacheIndex()
{
    ASSERT(CacheIndex == TLS_INVALID_INDEX);
    CacheIndex = CreateTLSIndex();
    return CacheIndex != TLS_INVALID_INDEX;
}

void FreeTCacheIndex()
{
    ASSERT(CacheIndex != TLS_INVALID_INDEX);
    TCache* cache = static_cast<TCache*>(GetTLSValue(CacheIndex));
    if (cache)
    {
        // Should we assert here? This means effectively that someone leaked a
        // compiler.
        //
        // WARN("Freeing thread-local TCache with %u references", cache->mRefCount);
        delete cache;
    }
    DestroyTLSIndex(CacheIndex);
    CacheIndex = TLS_INVALID_INDEX;
}

void TCache::initialize()
{
    ASSERT(CacheIndex != TLS_INVALID_INDEX && "Cache index not initialized");
    TCache* cache = static_cast<TCache*>(GetTLSValue(CacheIndex));
    if (!cache)
    {
        cache = new TCache();
        SetTLSValue(CacheIndex, cache);
    }

    cache->AddRef();
}

void TCache::destroy()
{
    ASSERT(CacheIndex != TLS_INVALID_INDEX && "Cache index not initialized");
    TCache* cache = static_cast<TCache*>(GetTLSValue(CacheIndex));
    ASSERT(cache && "Cache not initialized");
    if (cache->Release())
    {
        SetTLSValue(CacheIndex, nullptr);
    }
}

const TType *TCache::getType(TBasicType basicType,
                             TPrecision precision,
                             TQualifier qualifier,
                             unsigned char primarySize,
                             unsigned char secondarySize)
{
    ASSERT(CacheIndex != TLS_INVALID_INDEX && "TCache should be inititalized");
    TCache* cache = static_cast<TCache*>(GetTLSValue(CacheIndex));
    ASSERT(cache != nullptr && "CacheIndex should never have a null cache");

    TypeKey key(basicType, precision, qualifier,
                primarySize, secondarySize);
    auto it = cache->mTypes.find(key);
    if (it != cache->mTypes.end())
    {
        return it->second;
    }

    TScopedAllocator scopedAllocator(&cache->mAllocator);

    TType *type = new TType(basicType, precision, qualifier,
                            primarySize, secondarySize);
    type->realize();
    cache->mTypes.insert(std::make_pair(key, type));

    return type;
}
