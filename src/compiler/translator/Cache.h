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

#include "compiler/translator/Types.h"
#include "compiler/translator/PoolAlloc.h"

namespace sh
{

namespace TCache
{
    // Implementation

    template<TBasicType basicType,
             TPrecision precision,
             TQualifier qualifier,
             unsigned char primarySize = 1,
             unsigned char secondarySize = 1>
    const TType *getType()
    {
#ifdef __GNUC__
        static const char *name = __PRETTY_FUNCTION__;
#else
        static const char *name = __FUNCTION__;
#endif
        static const TType t(basicType, precision, qualifier, primarySize, secondarySize, name);
        return &t;
    }

    // Overloads

    template<TBasicType basicType,
             unsigned char primarySize = 1,
             unsigned char secondarySize = 1>
    const TType *getType1()
    {
        return getType<basicType, EbpUndefined, EvqGlobal, primarySize, secondarySize>();
    }

    template<TBasicType basicType,
             TQualifier qualifier,
             unsigned char primarySize = 1,
             unsigned char secondarySize = 1>
    const TType *getType2()
    {
        return getType<basicType, EbpUndefined, qualifier, primarySize, secondarySize>();
    }

    template<TBasicType basicType,
             TPrecision precision,
             TQualifier qualifier,
             unsigned char secondarySize>
    const TType *getTypeVecMatHelper(unsigned char primarySize)
    {
        static_assert(basicType == EbtFloat ||
                      basicType == EbtInt ||
                      basicType == EbtUInt ||
                      basicType == EbtBool, "unsupported basicType");
        switch (primarySize)
        {
            case 1: return getType<basicType, precision, qualifier, 1, secondarySize>();
            case 2: return getType<basicType, precision, qualifier, 2, secondarySize>();
            case 3: return getType<basicType, precision, qualifier, 3, secondarySize>();
            case 4: return getType<basicType, precision, qualifier, 4, secondarySize>();
            default: UNREACHABLE();
        }
    }

    template<TBasicType basicType,
             TPrecision precision = EbpUndefined,
             TQualifier qualifier = EvqGlobal>
    const TType *getTypeVecMat(unsigned char primarySize, unsigned char secondarySize = 1)
    {
        static_assert(basicType == EbtFloat ||
                      basicType == EbtInt ||
                      basicType == EbtUInt ||
                      basicType == EbtBool, "unsupported basicType");
        switch (secondarySize)
        {
            case 1: return getTypeVecMatHelper<basicType, precision, qualifier, 1>(primarySize);
            case 2: return getTypeVecMatHelper<basicType, precision, qualifier, 2>(primarySize);
            case 3: return getTypeVecMatHelper<basicType, precision, qualifier, 3>(primarySize);
            case 4: return getTypeVecMatHelper<basicType, precision, qualifier, 4>(primarySize);
            default: UNREACHABLE();
        }
    }

    template<TBasicType basicType,
             TPrecision precision = EbpUndefined>
    const TType *getTypeVecQualified(TQualifier qualifier, unsigned char size)
    {
        switch (qualifier) {
#define OP(Q) case Q: return getTypeVecMatHelper<basicType, precision, Q, 1>(size);
            TQUALIFIER(OP)
#undef OP
            default:
                UNREACHABLE();
        }
    }

    inline const TType *getTypeSimple(TBasicType basicType)
    {
        switch (basicType) {
#define OP(BT) case BT: return getType<BT, EbpUndefined, EvqGlobal, 1, 1>();
            TBASICTYPE(OP)
#undef OP
            default:
                UNREACHABLE();
        }
    }
} // namespace TCache

}  // namespace sh

#endif  // COMPILER_TRANSLATOR_CACHE_H_
