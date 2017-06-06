//
// Copyright 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ParamType:
//   Helper type for built-in function emulator tables. Defines types for parameters.

#ifndef COMPILER_TRANSLATOR_PARAMTYPE_H_
#define COMPILER_TRANSLATOR_PARAMTYPE_H_

#include "common/angleutils.h"
#include "compiler/translator/BaseTypes.h"

namespace sh
{

enum class ParamType : uint8_t
{
    Void,
    Bool1,
    Bool2,
    Bool3,
    Bool4,
    Float1,
    Float2,
    Float3,
    Float4,
    Int1,
    Int2,
    Int3,
    Int4,
    Mat2,
    Mat3,
    Mat4,
    Uint1,
    Uint2,
    Uint3,
    Uint4,
    Last,
};

constexpr TBasicType ParamToBasicType[] = {
    EbtVoid, EbtBool, EbtBool, EbtBool,  EbtBool,  EbtFloat, EbtFloat, EbtFloat, EbtFloat, EbtInt,
    EbtInt,  EbtInt,  EbtInt,  EbtFloat, EbtFloat, EbtFloat, EbtUInt,  EbtUInt,  EbtUInt,  EbtUInt,
};

constexpr int ParamTypePrimarySize[] = {
    1, 1, 2, 3, 4, 1, 2, 3, 4, 1, 2, 3, 4, 2, 3, 4, 1, 2, 3, 4,
};

constexpr int ParamTypeSecondarySize[] = {
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 3, 4, 1, 1, 1, 1,
};

constexpr size_t ParamTypeIndex(ParamType paramType)
{
    return static_cast<size_t>(paramType);
}

constexpr size_t NumParamTypes()
{
    return ParamTypeIndex(ParamType::Last);
}

static_assert(ArraySize(ParamToBasicType) == NumParamTypes(), "Invalid array size");
static_assert(ArraySize(ParamTypePrimarySize) == NumParamTypes(), "Invalid array size");
static_assert(ArraySize(ParamTypeSecondarySize) == NumParamTypes(), "Invalid array size");

constexpr TBasicType GetBasicType(ParamType paramType)
{
    return ParamToBasicType[ParamTypeIndex(paramType)];
}

constexpr int GetPrimarySize(ParamType paramType)
{
    return ParamTypePrimarySize[ParamTypeIndex(paramType)];
}

constexpr int GetSecondarySize(ParamType paramType)
{
    return ParamTypeSecondarySize[ParamTypeIndex(paramType)];
}

constexpr bool SameParamType(ParamType paramType,
                             TBasicType basicType,
                             int primarySize,
                             int secondarySize)
{
    return GetBasicType(paramType) == basicType && primarySize == GetPrimarySize(paramType) &&
           secondarySize == GetSecondarySize(paramType);
}

}  // namespace sh

#endif  // COMPILER_TRANSLATOR_PARAMTYPE_H_
