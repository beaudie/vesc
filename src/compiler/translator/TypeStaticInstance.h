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

// Size of the maximum possible constexpr-generated mangled name.
// If this value is too small, the compiler will produce errors.
static const size_t kStaticMangledNameMaxLength = 10;

// Type which holds the mangled names for constexpr-generated TTypes.
// This simple struct is needed so that a char array can be returned by value.
struct MangledName
{
    // If this array is too small, the compiler will produce errors.
    char name[kStaticMangledNameMaxLength + 1] = {};
};

// Generates a mangled name for a TType given its parameters.
constexpr MangledName BuildMangledName(
          TBasicType basicType,
          TPrecision precision,
          TQualifier qualifier,
          unsigned char primarySize,
          unsigned char secondarySize)
{
    MangledName name = {};
    char *n          = name.name;
    size_t at        = 0;
#define APPEND(SRC)                 \
    for (size_t i = 0; SRC[i]; ++i) \
    {                               \
        n[at++] = SRC[i];           \
    }                               \

    bool isMatrix = primarySize > 1 && secondarySize > 1;
    bool isVector = primarySize > 1 && secondarySize == 1;

    if (isMatrix)
    {
        n[at++] = 'm';
    }
    else if (isVector)
    {
        n[at++] = 'v';
    }

    switch (basicType)
    {
        case EbtFloat:
            n[at++] = 'f';
            break;
        case EbtInt:
            n[at++] = 'i';
            break;
        case EbtUInt:
            n[at++] = 'u';
            break;
        case EbtBool:
            n[at++] = 'b';
            break;
        case EbtYuvCscStandardEXT:
            APPEND("ycs");
            break;
        case EbtSampler2D:
            APPEND("s2");
            break;
        case EbtSampler3D:
            APPEND("s3");
            break;
        case EbtSamplerCube:
            APPEND("sC");
            break;
        case EbtSampler2DArray:
            APPEND("s2a");
            break;
        case EbtSamplerExternalOES:
            APPEND("sext");
            break;
        case EbtSamplerExternal2DY2YEXT:
            APPEND("sext2y2y");
            break;
        case EbtSampler2DRect:
            APPEND("s2r");
            break;
        case EbtSampler2DMS:
            APPEND("s2ms");
            break;
        case EbtISampler2D:
            APPEND("is2");
            break;
        case EbtISampler3D:
            APPEND("is3");
            break;
        case EbtISamplerCube:
            APPEND("isC");
            break;
        case EbtISampler2DArray:
            APPEND("is2a");
            break;
        case EbtISampler2DMS:
            APPEND("is2ms");
            break;
        case EbtUSampler2D:
            APPEND("us2");
            break;
        case EbtUSampler3D:
            APPEND("us3");
            break;
        case EbtUSamplerCube:
            APPEND("usC");
            break;
        case EbtUSampler2DArray:
            APPEND("us2a");
            break;
        case EbtUSampler2DMS:
            APPEND("us2ms");
            break;
        case EbtSampler2DShadow:
            APPEND("s2s");
            break;
        case EbtSamplerCubeShadow:
            APPEND("sCs");
            break;
        case EbtSampler2DArrayShadow:
            APPEND("s2as");
            break;
        case EbtImage2D:
            APPEND("im2");
            break;
        case EbtIImage2D:
            APPEND("iim2");
            break;
        case EbtUImage2D:
            APPEND("uim2");
            break;
        case EbtImage3D:
            APPEND("im3");
            break;
        case EbtIImage3D:
            APPEND("iim3");
            break;
        case EbtUImage3D:
            APPEND("uim3");
            break;
        case EbtImage2DArray:
            APPEND("im2a");
            break;
        case EbtIImage2DArray:
            APPEND("iim2a");
            break;
        case EbtUImage2DArray:
            APPEND("uim2a");
            break;
        case EbtImageCube:
            APPEND("imc");
            break;
        case EbtIImageCube:
            APPEND("iimc");
            break;
        case EbtUImageCube:
            APPEND("uimc");
            break;
        case EbtAtomicCounter:
            APPEND("ac");
            break;
        default:
            break;
    }

    n[at++] = '0' + primarySize;
    if (isMatrix)
    {
        n[at++] = 'x';
        n[at++] = '0' + secondarySize;
    }

    n[at++] = ';';

    n[at] = '\0';
    return name;
#undef APPEND
}

// This "variable" contains the mangled names for every constexpr-generated TType.
template <TBasicType basicType,
          TPrecision precision,
          TQualifier qualifier,
          unsigned char primarySize,
          unsigned char secondarySize>
static constexpr MangledName kMangledNameInstance =
    BuildMangledName(basicType, precision, qualifier, primarySize, secondarySize);

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
