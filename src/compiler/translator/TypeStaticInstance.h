//
// Copyright (c) 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef COMPILER_TRANSLATOR_TYPE_STATIC_INSTANCE_H
#define COMPILER_TRANSLATOR_TYPE_STATIC_INSTANCE_H

#include "compiler/translator/Types.h"

namespace sh
{

namespace TTypeStaticInstance
{

namespace Helpers
{

// This "variable" contains the mangled names for every constexpr-generated TType.
template <TBasicType basicType,
          TPrecision precision,
          TQualifier qualifier,
          unsigned char primarySize,
          unsigned char secondarySize>
static constexpr TType::StaticMangledName kMangledNameInstance =
    TType::BuildStaticMangledName(basicType, precision, qualifier, primarySize, secondarySize);

// This "variable" contains every constexpr-generated TType.
template <TBasicType basicType,
          TPrecision precision,
          TQualifier qualifier,
          unsigned char primarySize,
          unsigned char secondarySize>
static constexpr TType Instance =
    TType(basicType,
          precision,
          qualifier,
          primarySize,
          secondarySize,
          kMangledNameInstance<basicType, precision, qualifier, primarySize, secondarySize>.name);

}  // namespace Helpers


template <TBasicType basicType,
          TPrecision precision,
          TQualifier qualifier,
          unsigned char primarySize,
          unsigned char secondarySize>
constexpr const TType *Get()
{
    static_assert(1 <= primarySize && primarySize <= 4, "primarySize out of bounds");
    static_assert(1 <= secondarySize && secondarySize <= 4, "secondarySize out of bounds");
    return &Helpers::Instance<basicType, precision, qualifier, primarySize, secondarySize>;
}

// Overloads

template <TBasicType basicType, unsigned char primarySize = 1, unsigned char secondarySize = 1>
constexpr const TType *Get1()
{
    return Get<basicType, EbpUndefined, EvqGlobal, primarySize, secondarySize>();
}

template <TBasicType basicType,
          TQualifier qualifier,
          unsigned char primarySize   = 1,
          unsigned char secondarySize = 1>
const TType *Get2()
{
    return Get<basicType, EbpUndefined, qualifier, primarySize, secondarySize>();
}

// Runtime indirection (converts runtime values to template args)

namespace Helpers
{

// Helper which takes secondarySize statically but primarySize dynamically.
template <TBasicType basicType,
          TPrecision precision,
          TQualifier qualifier,
          unsigned char secondarySize>
const TType *GetForVecMatHelper(unsigned char primarySize)
{
    static_assert(basicType == EbtFloat || basicType == EbtInt || basicType == EbtUInt ||
                      basicType == EbtBool,
                  "unsupported basicType");
    switch (primarySize)
    {
        case 1:
            return Get<basicType, precision, qualifier, 1, secondarySize>();
        case 2:
            return Get<basicType, precision, qualifier, 2, secondarySize>();
        case 3:
            return Get<basicType, precision, qualifier, 3, secondarySize>();
        case 4:
            return Get<basicType, precision, qualifier, 4, secondarySize>();
        default:
            UNREACHABLE();
            return Get1<EbtVoid>();
    }
}

}  // namespace Helpers

template <TBasicType basicType,
          TPrecision precision = EbpUndefined,
          TQualifier qualifier = EvqGlobal>
const TType *GetForVecMat(unsigned char primarySize, unsigned char secondarySize = 1)
{
    static_assert(basicType == EbtFloat || basicType == EbtInt || basicType == EbtUInt ||
                      basicType == EbtBool,
                  "unsupported basicType");
    switch (secondarySize)
    {
        case 1:
            return Helpers::GetForVecMatHelper<basicType, precision, qualifier, 1>(primarySize);
        case 2:
            return Helpers::GetForVecMatHelper<basicType, precision, qualifier, 2>(primarySize);
        case 3:
            return Helpers::GetForVecMatHelper<basicType, precision, qualifier, 3>(primarySize);
        case 4:
            return Helpers::GetForVecMatHelper<basicType, precision, qualifier, 4>(primarySize);
        default:
            UNREACHABLE();
            return Get1<EbtVoid>();
    }
}

template <TBasicType basicType, TPrecision precision = EbpUndefined>
const TType *GetForVec(TQualifier qualifier, unsigned char size)
{
    switch (qualifier)
    {
        case EvqGlobal:
            return Helpers::GetForVecMatHelper<basicType, precision, EvqGlobal, 1>(size);
        case EvqOut:
            return Helpers::GetForVecMatHelper<basicType, precision, EvqOut, 1>(size);
        default:
            UNREACHABLE();
            return Get1<EbtVoid>();
    }
}

const TType *GetForFloatImage(TBasicType basicType);

const TType *GetForIntImage(TBasicType basicType);

const TType *GetForUintImage(TBasicType basicType);

}  // namespace TTypeStaticInstance

}  // namespace sh

#endif  // COMPILER_TRANSLATOR_TYPE_STATIC_INSTANCE_H
