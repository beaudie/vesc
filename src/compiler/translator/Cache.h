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

struct MangledName
{
    // If this array is too small, the compiler will produce errors.
    char name[11] = {};
};

template <TBasicType basicType,
          TPrecision precision,
          TQualifier qualifier,
          unsigned char primarySize,
          unsigned char secondarySize>
constexpr MangledName getStaticMangledName()
{
    MangledName name = {};
    char *n          = name.name;
    size_t at        = 0;
#define APPEND(SRC)                     \
    do                                  \
    {                                   \
        const char *src = (SRC);        \
        for (size_t i = 0; src[i]; ++i) \
        {                               \
            n[at++] = src[i];           \
        }                               \
    } while (0)

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

template <TBasicType basicType,
          TPrecision precision,
          TQualifier qualifier,
          unsigned char primarySize,
          unsigned char secondarySize>
static constexpr MangledName mangledNameInstance =
    getStaticMangledName<basicType, precision, qualifier, primarySize, secondarySize>();

// Implementation

template <TBasicType basicType,
          TPrecision precision,
          TQualifier qualifier,
          unsigned char primarySize,
          unsigned char secondarySize>
static constexpr TType getTypeValue =
    TType(basicType,
          precision,
          qualifier,
          primarySize,
          secondarySize,
          mangledNameInstance<basicType, precision, qualifier, primarySize, secondarySize>.name);

template <TBasicType basicType,
          TPrecision precision,
          TQualifier qualifier,
          unsigned char primarySize,
          unsigned char secondarySize>
constexpr const TType *getType()
{
    static_assert(1 <= primarySize && primarySize <= 4, "primarySize out of bounds");
    static_assert(1 <= secondarySize && secondarySize <= 4, "secondarySize out of bounds");
    return &getTypeValue<basicType, precision, qualifier, primarySize, secondarySize>;
}

// Overloads

template <TBasicType basicType, unsigned char primarySize = 1, unsigned char secondarySize = 1>
constexpr const TType *getType1()
{
    return getType<basicType, EbpUndefined, EvqGlobal, primarySize, secondarySize>();
}

template <TBasicType basicType,
          TQualifier qualifier,
          unsigned char primarySize   = 1,
          unsigned char secondarySize = 1>
const TType *getType2()
{
    return getType<basicType, EbpUndefined, qualifier, primarySize, secondarySize>();
}

// Dynamic indirection (converts runtime values to template args)

template <TBasicType basicType,
          TPrecision precision,
          TQualifier qualifier,
          unsigned char secondarySize>
const TType *getTypeVecMatHelper(unsigned char primarySize)
{
    static_assert(basicType == EbtFloat || basicType == EbtInt || basicType == EbtUInt ||
                      basicType == EbtBool,
                  "unsupported basicType");
    switch (primarySize)
    {
        case 1:
            return getType<basicType, precision, qualifier, 1, secondarySize>();
        case 2:
            return getType<basicType, precision, qualifier, 2, secondarySize>();
        case 3:
            return getType<basicType, precision, qualifier, 3, secondarySize>();
        case 4:
            return getType<basicType, precision, qualifier, 4, secondarySize>();
        default:
            UNREACHABLE();
            return getType1<EbtVoid>();
    }
}

template <TBasicType basicType,
          TPrecision precision = EbpUndefined,
          TQualifier qualifier = EvqGlobal>
const TType *getTypeVecMat(unsigned char primarySize, unsigned char secondarySize = 1)
{
    static_assert(basicType == EbtFloat || basicType == EbtInt || basicType == EbtUInt ||
                      basicType == EbtBool,
                  "unsupported basicType");
    switch (secondarySize)
    {
        case 1:
            return getTypeVecMatHelper<basicType, precision, qualifier, 1>(primarySize);
        case 2:
            return getTypeVecMatHelper<basicType, precision, qualifier, 2>(primarySize);
        case 3:
            return getTypeVecMatHelper<basicType, precision, qualifier, 3>(primarySize);
        case 4:
            return getTypeVecMatHelper<basicType, precision, qualifier, 4>(primarySize);
        default:
            UNREACHABLE();
            return getType1<EbtVoid>();
    }
}

template <TBasicType basicType, TPrecision precision = EbpUndefined>
const TType *getTypeVecQualified(TQualifier qualifier, unsigned char size)
{
    switch (qualifier)
    {
        case EvqGlobal:
            return getTypeVecMatHelper<basicType, precision, EvqGlobal, 1>(size);
        case EvqOut:
            return getTypeVecMatHelper<basicType, precision, EvqOut, 1>(size);
        default:
            UNREACHABLE();
            return getType1<EbtVoid>();
    }
}

inline const TType *getFloatImageType(TBasicType basicType)
{
    switch (basicType)
    {
        case EbtGImage2D:
            return getType<EbtImage2D, EbpUndefined, EvqGlobal, 1, 1>();
        case EbtGImage3D:
            return getType<EbtImage3D, EbpUndefined, EvqGlobal, 1, 1>();
        case EbtGImage2DArray:
            return getType<EbtImage2DArray, EbpUndefined, EvqGlobal, 1, 1>();
        case EbtGImageCube:
            return getType<EbtImageCube, EbpUndefined, EvqGlobal, 1, 1>();
        default:
            UNREACHABLE();
            return getType1<EbtVoid>();
    }
}

inline const TType *getIntImageType(TBasicType basicType)
{
    switch (basicType)
    {
        case EbtGImage2D:
            return getType<EbtIImage2D, EbpUndefined, EvqGlobal, 1, 1>();
        case EbtGImage3D:
            return getType<EbtIImage3D, EbpUndefined, EvqGlobal, 1, 1>();
        case EbtGImage2DArray:
            return getType<EbtIImage2DArray, EbpUndefined, EvqGlobal, 1, 1>();
        case EbtGImageCube:
            return getType<EbtIImageCube, EbpUndefined, EvqGlobal, 1, 1>();
        default:
            UNREACHABLE();
            return getType1<EbtVoid>();
    }
}

inline const TType *getUintImageType(TBasicType basicType)
{
    switch (basicType)
    {
        case EbtGImage2D:
            return getType<EbtUImage2D, EbpUndefined, EvqGlobal, 1, 1>();
        case EbtGImage3D:
            return getType<EbtUImage3D, EbpUndefined, EvqGlobal, 1, 1>();
        case EbtGImage2DArray:
            return getType<EbtUImage2DArray, EbpUndefined, EvqGlobal, 1, 1>();
        case EbtGImageCube:
            return getType<EbtUImageCube, EbpUndefined, EvqGlobal, 1, 1>();
        default:
            UNREACHABLE();
            return getType1<EbtVoid>();
    }
}

}  // namespace TCache

}  // namespace sh

#endif  // COMPILER_TRANSLATOR_CACHE_H_
