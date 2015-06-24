//
// Copyright (c) 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// Cache.cpp: Implements a cache for various commonly created objects.

#include "common/angleutils.h"
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

TCache *TCache::sCache = nullptr;

void TCache::initialize()
{
    if (sCache == nullptr)
    {
        sCache = new TCache();
    }
}

void TCache::destroy()
{
    SafeDelete(sCache);
}

TCache::TypeKey TCache::makeTypeKey(TBasicType basicType,
                                    TPrecision precision,
                                    TQualifier qualifier,
                                    unsigned char primarySize,
                                    unsigned char secondarySize)
{
    const size_t kBasicTypeBits = 6; // see EbtLast
    const size_t kPrecisionBits = 3; // see EbpLast
    const size_t kQualifierBits = 6; // see EvqLast
    const size_t kPrimarySizeBits = 8;
    const size_t kSecondarySizeBits = 8;
    const size_t kTotalBits =
        kBasicTypeBits + kPrecisionBits + kQualifierBits +
        kPrimarySizeBits + kSecondarySizeBits;
    // Note that kTotalBits is 31, so there is one spare bit.
    // If you need more, you'll need to change TypeKey to uint64_t.

    static_assert(((1<<kBasicTypeBits) - 1) >= EbtLast,
                  "kBasicTypeBits is not enough to store TBasicType");

    static_assert(((1<<kPrecisionBits) - 1) >= EbpLast,
                  "kPrecisionBits is not enough to store TPrecision");

    static_assert(((1<<kQualifierBits) - 1) >= EvqLast,
                  "kQualifierBits is not enough to store TQualifier");

    static_assert(kPrimarySizeBits >= (sizeof(primarySize) * CHAR_BIT),
                  "kPrimarySizeBits is not enough to store primarySize");

    static_assert(kSecondarySizeBits >= (sizeof(secondarySize) * CHAR_BIT),
                  "kSecondarySizeBits is not enough to store secondarySize");

    static_assert(sizeof(TypeKey) * CHAR_BIT >= kTotalBits,
                  "TypeKey is too small to hold all the components");

    TypeKey key = 0;
    size_t shift = 0;

    // Bitfields are too underspecified to be used reliably, so
    // we have to pack manually.
    #define PACK_KEY_COMPONENT(component, bits) \
        key <<= shift; \
        key |= component & ((1<<bits) - 1); \
        shift = bits

    PACK_KEY_COMPONENT(basicType, kBasicTypeBits);
    PACK_KEY_COMPONENT(precision, kPrecisionBits);
    PACK_KEY_COMPONENT(qualifier, kQualifierBits);
    PACK_KEY_COMPONENT(primarySize, kPrimarySizeBits);
    PACK_KEY_COMPONENT(secondarySize, kSecondarySizeBits);

    #undef PACK_KEY_COMPONENT

    return key;
}

const TType *TCache::getType(TBasicType basicType,
                             TPrecision precision,
                             TQualifier qualifier,
                             unsigned char primarySize,
                             unsigned char secondarySize)
{
    TypeKey key = makeTypeKey(basicType, precision, qualifier,
                              primarySize, secondarySize);
    auto it = sCache->mTypes.find(key);
    if (it != sCache->mTypes.end())
    {
        return it->second;
    }

    TScopedAllocator scopedAllocator(&sCache->mAllocator);

    TType *type = new TType(basicType, precision, qualifier,
                            primarySize, secondarySize);
    type->realize();
    sCache->mTypes.insert(std::make_pair(key, type));

    return type;
}
