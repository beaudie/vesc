//
// Copyright (c) 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// Cache.h: Implements a cache for various commonly created objects.

#ifndef COMPILER_TRANSLATOR_CACHE_H_
#define COMPILER_TRANSLATOR_CACHE_H_

#include <stdint.h>
#include <string.h>
#include <map>

#include "compiler/translator/Types.h"
#include "compiler/translator/PoolAlloc.h"

class TCache
{
  public:

    static void initialize();
    static void destroy();

    static const TType *getType(TBasicType basicType,
                                TPrecision precision)
    {
        return getType(basicType, precision, EvqTemporary,
                       1, 1);
    }
    static const TType *getType(TBasicType basicType,
                                unsigned char primarySize = 1,
                                unsigned char secondarySize = 1)
    {
        return getType(basicType, EbpUndefined, EvqGlobal,
                       primarySize, secondarySize);
    }
    static const TType *getType(TBasicType basicType,
                                TQualifier qualifier,
                                unsigned char primarySize = 1,
                                unsigned char secondarySize = 1)
    {
        return getType(basicType, EbpUndefined, qualifier,
                       primarySize, secondarySize);
    }
    static const TType *getType(TBasicType basicType,
                                TPrecision precision,
                                TQualifier qualifier,
                                unsigned char primarySize,
                                unsigned char secondarySize);

  private:
    TCache()
        : mRefCount(0)
    {}

    union TypeKey
    {
        TypeKey(TBasicType basicType,
                TPrecision precision,
                TQualifier qualifier,
                unsigned char primarySize,
                unsigned char secondarySize);

        typedef uint8_t EnumComponentType;
        struct
        {
            EnumComponentType basicType;
            EnumComponentType precision;
            EnumComponentType qualifier;
            unsigned char primarySize;
            unsigned char secondarySize;
        } components;
        uint64_t value;

        bool operator < (const TypeKey &other) const
        {
            return value < other.value;
        }
    };
    typedef std::map<TypeKey, const TType*> TypeMap;

    TypeMap mTypes;
    TPoolAllocator mAllocator;
    uint32_t mRefCount;

    void AddRef() {
        assert(mRefCount != UINT32_MAX && "Preventing thread-local cache refcount overflow");
        ++mRefCount;
    }

    /**
     * Returns true if the cache was just destroyed because the reference count
     * reached 0.
     */
    bool Release() {
        assert(mRefCount != 0);
        if (--mRefCount == 0) {
            delete this;
            return true;
        }
        return false;
    }
};

#endif  // COMPILER_TRANSLATOR_CACHE_H_
