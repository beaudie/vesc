//
// Copyright (c) 2002-2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

//
// Create strings that declare built-in definitions, add built-ins that
// cannot be expressed in the files, and establish mappings between 
// built-in functions and operators.
//

#include "compiler/translator/Initialize.h"

#include "compiler/translator/intermediate.h"

void InsertBuiltInFunctions(ShShaderType type, ShShaderSpec spec, const ShBuiltInResources &resources, TSymbolTable &symbolTable)
{
    TType *float1 = new TType(EbtFloat, EbpUndefined, EvqGlobal, 1);
    TType *float2 = new TType(EbtFloat, EbpUndefined, EvqGlobal, 2);
    TType *float3 = new TType(EbtFloat, EbpUndefined, EvqGlobal, 3);
    TType *float4 = new TType(EbtFloat, EbpUndefined, EvqGlobal, 4);

    TType *int1 = new TType(EbtInt, EbpUndefined, EvqGlobal, 1);
    TType *int2 = new TType(EbtInt, EbpUndefined, EvqGlobal, 2);
    TType *int3 = new TType(EbtInt, EbpUndefined, EvqGlobal, 3);
    TType *int4 = new TType(EbtInt, EbpUndefined, EvqGlobal, 4);

    //
    // Angle and Trigonometric Functions.
    //
    symbolTable.insertBuiltIn(COMMON_BUILTINS, float1, "radians", float1);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, float2, "radians", float2);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, float3, "radians", float3);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, float4, "radians", float4);

    symbolTable.insertBuiltIn(COMMON_BUILTINS, float1, "degrees", float1);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, float2, "degrees", float2);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, float3, "degrees", float3);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, float4, "degrees", float4);

    symbolTable.insertBuiltIn(COMMON_BUILTINS, float1, "sin", float1);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, float2, "sin", float2);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, float3, "sin", float3);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, float4, "sin", float4);

    symbolTable.insertBuiltIn(COMMON_BUILTINS, float1, "cos", float1);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, float2, "cos", float2);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, float3, "cos", float3);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, float4, "cos", float4);

    symbolTable.insertBuiltIn(COMMON_BUILTINS, float1, "tan", float1);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, float2, "tan", float2);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, float3, "tan", float3);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, float4, "tan", float4);

    symbolTable.insertBuiltIn(COMMON_BUILTINS, float1, "asin", float1);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, float2, "asin", float2);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, float3, "asin", float3);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, float4, "asin", float4);

    symbolTable.insertBuiltIn(COMMON_BUILTINS, float1, "acos", float1);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, float2, "acos", float2);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, float3, "acos", float3);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, float4, "acos", float4);

    symbolTable.insertBuiltIn(COMMON_BUILTINS, float1, "atan", float1, float1);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, float2, "atan", float2, float2);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, float3, "atan", float3, float3);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, float4, "atan", float4, float4);

    symbolTable.insertBuiltIn(COMMON_BUILTINS, float1, "atan", float1);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, float2, "atan", float2);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, float3, "atan", float3);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, float4, "atan", float4);

    //
    // Exponential Functions.
    //
    symbolTable.insertBuiltIn(COMMON_BUILTINS, float1, "pow", float1, float1);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, float2, "pow", float2, float2);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, float3, "pow", float3, float3);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, float4, "pow", float4, float4);

    symbolTable.insertBuiltIn(COMMON_BUILTINS, float1, "exp", float1);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, float2, "exp", float2);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, float3, "exp", float3);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, float4, "exp", float4);

    symbolTable.insertBuiltIn(COMMON_BUILTINS, float1, "log", float1);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, float2, "log", float2);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, float3, "log", float3);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, float4, "log", float4);

    symbolTable.insertBuiltIn(COMMON_BUILTINS, float1, "exp2", float1);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, float2, "exp2", float2);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, float3, "exp2", float3);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, float4, "exp2", float4);

    symbolTable.insertBuiltIn(COMMON_BUILTINS, float1, "log2", float1);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, float2, "log2", float2);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, float3, "log2", float3);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, float4, "log2", float4);

    symbolTable.insertBuiltIn(COMMON_BUILTINS, float1, "sqrt", float1);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, float2, "sqrt", float2);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, float3, "sqrt", float3);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, float4, "sqrt", float4);

    symbolTable.insertBuiltIn(COMMON_BUILTINS, float1, "inversesqrt", float1);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, float2, "inversesqrt", float2);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, float3, "inversesqrt", float3);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, float4, "inversesqrt", float4);

    //
    // Common Functions.
    //
    symbolTable.insertBuiltIn(COMMON_BUILTINS, float1, "abs", float1);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, float2, "abs", float2);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, float3, "abs", float3);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, float4, "abs", float4);

    symbolTable.insertBuiltIn(COMMON_BUILTINS, float1, "sign", float1);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, float2, "sign", float2);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, float3, "sign", float3);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, float4, "sign", float4);

    symbolTable.insertBuiltIn(COMMON_BUILTINS, float1, "floor", float1);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, float2, "floor", float2);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, float3, "floor", float3);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, float4, "floor", float4);

    symbolTable.insertBuiltIn(COMMON_BUILTINS, float1, "ceil", float1);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, float2, "ceil", float2);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, float3, "ceil", float3);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, float4, "ceil", float4);

    symbolTable.insertBuiltIn(COMMON_BUILTINS, float1, "fract", float1);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, float2, "fract", float2);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, float3, "fract", float3);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, float4, "fract", float4);

    symbolTable.insertBuiltIn(COMMON_BUILTINS, float1, "mod", float1, float1);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, float2, "mod", float2, float1);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, float3, "mod", float3, float1);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, float4, "mod", float4, float1);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, float2, "mod", float2, float2);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, float3, "mod", float3, float3);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, float4, "mod", float4, float4);

    symbolTable.insertBuiltIn(COMMON_BUILTINS, float1, "min", float1, float1);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, float2, "min", float2, float1);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, float3, "min", float3, float1);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, float4, "min", float4, float1);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, float2, "min", float2, float2);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, float3, "min", float3, float3);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, float4, "min", float4, float4);

    symbolTable.insertBuiltIn(COMMON_BUILTINS, float1, "max", float1, float1);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, float2, "max", float2, float1);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, float3, "max", float3, float1);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, float4, "max", float4, float1);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, float2, "max", float2, float2);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, float3, "max", float3, float3);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, float4, "max", float4, float4);

    symbolTable.insertBuiltIn(COMMON_BUILTINS, float1, "clamp", float1, float1, float1);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, float2, "clamp", float2, float1, float1);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, float3, "clamp", float3, float1, float1);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, float4, "clamp", float4, float1, float1);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, float2, "clamp", float2, float2, float2);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, float3, "clamp", float3, float3, float3);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, float4, "clamp", float4, float4, float4);

    symbolTable.insertBuiltIn(COMMON_BUILTINS, float1, "mix", float1, float1, float1);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, float2, "mix", float2, float2, float1);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, float3, "mix", float3, float3, float1);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, float4, "mix", float4, float4, float1);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, float2, "mix", float2, float2, float2);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, float3, "mix", float3, float3, float3);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, float4, "mix", float4, float4, float4);

    symbolTable.insertBuiltIn(COMMON_BUILTINS, float1, "step", float1, float1);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, float2, "step", float2, float2);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, float3, "step", float3, float3);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, float4, "step", float4, float4);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, float2, "step", float1, float2);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, float3, "step", float1, float3);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, float4, "step", float1, float4);

    symbolTable.insertBuiltIn(COMMON_BUILTINS, float1, "smoothstep", float1, float1, float1);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, float2, "smoothstep", float2, float2, float2);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, float3, "smoothstep", float3, float3, float3);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, float4, "smoothstep", float4, float4, float4);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, float2, "smoothstep", float1, float1, float2);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, float3, "smoothstep", float1, float1, float3);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, float4, "smoothstep", float1, float1, float4);

    //
    // Geometric Functions.
    //
    symbolTable.insertBuiltIn(COMMON_BUILTINS, float1, "length", float1);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, float1, "length", float2);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, float1, "length", float3);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, float1, "length", float4);

    symbolTable.insertBuiltIn(COMMON_BUILTINS, float1, "distance", float1, float1);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, float1, "distance", float2, float2);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, float1, "distance", float3, float3);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, float1, "distance", float4, float4);

    symbolTable.insertBuiltIn(COMMON_BUILTINS, float1, "dot", float1, float1);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, float1, "dot", float2, float2);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, float1, "dot", float3, float3);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, float1, "dot", float4, float4);

    symbolTable.insertBuiltIn(COMMON_BUILTINS, float3, "cross", float3, float3);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, float1, "normalize", float1);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, float2, "normalize", float2);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, float3, "normalize", float3);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, float4, "normalize", float4);

    symbolTable.insertBuiltIn(COMMON_BUILTINS, float1, "faceforward", float1, float1, float1);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, float2, "faceforward", float2, float2, float2);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, float3, "faceforward", float3, float3, float3);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, float4, "faceforward", float4, float4, float4);

    symbolTable.insertBuiltIn(COMMON_BUILTINS, float1, "reflect", float1, float1);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, float2, "reflect", float2, float2);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, float3, "reflect", float3, float3);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, float4, "reflect", float4, float4);

    symbolTable.insertBuiltIn(COMMON_BUILTINS, float1, "refract", float1, float1, float1);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, float2, "refract", float2, float2, float1);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, float3, "refract", float3, float3, float1);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, float4, "refract", float4, float4, float1);

    TType *mat2 = new TType(EbtFloat, EbpUndefined, EvqGlobal, 2, 2);
    TType *mat3 = new TType(EbtFloat, EbpUndefined, EvqGlobal, 3, 3);
    TType *mat4 = new TType(EbtFloat, EbpUndefined, EvqGlobal, 4, 4);

    //
    // Matrix Functions.
    //
    symbolTable.insertBuiltIn(COMMON_BUILTINS, mat2, "matrixCompMult", mat2, mat2);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, mat3, "matrixCompMult", mat3, mat3);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, mat4, "matrixCompMult", mat4, mat4);

    TType *bool1 = new TType(EbtBool, EbpUndefined, EvqGlobal, 1);
    TType *bool2 = new TType(EbtBool, EbpUndefined, EvqGlobal, 2);
    TType *bool3 = new TType(EbtBool, EbpUndefined, EvqGlobal, 3);
    TType *bool4 = new TType(EbtBool, EbpUndefined, EvqGlobal, 4);

    //
    // Vector relational functions.
    //
    symbolTable.insertBuiltIn(COMMON_BUILTINS, bool2, "lessThan", float2, float2);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, bool3, "lessThan", float3, float3);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, bool4, "lessThan", float4, float4);

    symbolTable.insertBuiltIn(COMMON_BUILTINS, bool2, "lessThan", int2, int2);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, bool3, "lessThan", int3, int3);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, bool4, "lessThan", int4, int4);

    symbolTable.insertBuiltIn(COMMON_BUILTINS, bool2, "lessThanEqual", float2, float2);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, bool3, "lessThanEqual", float3, float3);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, bool4, "lessThanEqual", float4, float4);

    symbolTable.insertBuiltIn(COMMON_BUILTINS, bool2, "lessThanEqual", int2, int2);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, bool3, "lessThanEqual", int3, int3);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, bool4, "lessThanEqual", int4, int4);

    symbolTable.insertBuiltIn(COMMON_BUILTINS, bool2, "greaterThan", float2, float2);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, bool3, "greaterThan", float3, float3);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, bool4, "greaterThan", float4, float4);

    symbolTable.insertBuiltIn(COMMON_BUILTINS, bool2, "greaterThan", int2, int2);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, bool3, "greaterThan", int3, int3);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, bool4, "greaterThan", int4, int4);

    symbolTable.insertBuiltIn(COMMON_BUILTINS, bool2, "greaterThanEqual", float2, float2);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, bool3, "greaterThanEqual", float3, float3);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, bool4, "greaterThanEqual", float4, float4);

    symbolTable.insertBuiltIn(COMMON_BUILTINS, bool2, "greaterThanEqual", int2, int2);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, bool3, "greaterThanEqual", int3, int3);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, bool4, "greaterThanEqual", int4, int4);

    symbolTable.insertBuiltIn(COMMON_BUILTINS, bool2, "equal", float2, float2);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, bool3, "equal", float3, float3);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, bool4, "equal", float4, float4);

    symbolTable.insertBuiltIn(COMMON_BUILTINS, bool2, "equal", int2, int2);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, bool3, "equal", int3, int3);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, bool4, "equal", int4, int4);

    symbolTable.insertBuiltIn(COMMON_BUILTINS, bool2, "equal", bool2, bool2);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, bool3, "equal", bool3, bool3);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, bool4, "equal", bool4, bool4);

    symbolTable.insertBuiltIn(COMMON_BUILTINS, bool2, "notEqual", float2, float2);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, bool3, "notEqual", float3, float3);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, bool4, "notEqual", float4, float4);

    symbolTable.insertBuiltIn(COMMON_BUILTINS, bool2, "notEqual", int2, int2);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, bool3, "notEqual", int3, int3);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, bool4, "notEqual", int4, int4);

    symbolTable.insertBuiltIn(COMMON_BUILTINS, bool2, "notEqual", bool2, bool2);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, bool3, "notEqual", bool3, bool3);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, bool4, "notEqual", bool4, bool4);

    symbolTable.insertBuiltIn(COMMON_BUILTINS, bool1, "any", bool2);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, bool1, "any", bool3);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, bool1, "any", bool4);

    symbolTable.insertBuiltIn(COMMON_BUILTINS, bool1, "all", bool2);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, bool1, "all", bool3);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, bool1, "all", bool4);

    symbolTable.insertBuiltIn(COMMON_BUILTINS, bool2, "not", bool2);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, bool3, "not", bool3);
    symbolTable.insertBuiltIn(COMMON_BUILTINS, bool4, "not", bool4);

    TType *sampler2D = new TType(EbtSampler2D, EbpUndefined, EvqGlobal, 1);
    TType *samplerCube = new TType(EbtSamplerCube, EbpUndefined, EvqGlobal, 1);

    //
    // Texture Functions for GLSL ES 1.0
    //
    symbolTable.insertBuiltIn(ESSL1_BUILTINS, float4, "texture2D", sampler2D, float2);
    symbolTable.insertBuiltIn(ESSL1_BUILTINS, float4, "texture2DProj", sampler2D, float3);
    symbolTable.insertBuiltIn(ESSL1_BUILTINS, float4, "texture2DProj", sampler2D, float4);
    symbolTable.insertBuiltIn(ESSL1_BUILTINS, float4, "textureCube", samplerCube, float3);

    if (resources.OES_EGL_image_external)
    {
        TType *samplerExternalOES = new TType(EbtSamplerExternalOES, EbpUndefined, EvqGlobal, 1);

        symbolTable.insertBuiltIn(ESSL1_BUILTINS, float4, "texture2D", samplerExternalOES, float2);
        symbolTable.insertBuiltIn(ESSL1_BUILTINS, float4, "texture2DProj", samplerExternalOES, float3);
        symbolTable.insertBuiltIn(ESSL1_BUILTINS, float4, "texture2DProj", samplerExternalOES, float4);
    }

    if (resources.ARB_texture_rectangle)
    {
        TType *sampler2DRect = new TType(EbtSampler2DRect, EbpUndefined, EvqGlobal, 1);

        symbolTable.insertBuiltIn(ESSL1_BUILTINS, float4, "texture2DRect", sampler2DRect, float2);
        symbolTable.insertBuiltIn(ESSL1_BUILTINS, float4, "texture2DRectProj", sampler2DRect, float3);
        symbolTable.insertBuiltIn(ESSL1_BUILTINS, float4, "texture2DRectProj", sampler2DRect, float4);
    }

    if (type == SH_FRAGMENT_SHADER)
    {
        symbolTable.insertBuiltIn(ESSL1_BUILTINS, float4, "texture2D", sampler2D, float2, float1);
        symbolTable.insertBuiltIn(ESSL1_BUILTINS, float4, "texture2DProj", sampler2D, float3, float1);
        symbolTable.insertBuiltIn(ESSL1_BUILTINS, float4, "texture2DProj", sampler2D, float4, float1);
        symbolTable.insertBuiltIn(ESSL1_BUILTINS, float4, "textureCube", samplerCube, float3, float1);

        if (resources.OES_standard_derivatives)
        {
            symbolTable.insertBuiltIn(ESSL1_BUILTINS, float1, "dFdx", float1);
            symbolTable.insertBuiltIn(ESSL1_BUILTINS, float2, "dFdx", float2);
            symbolTable.insertBuiltIn(ESSL1_BUILTINS, float3, "dFdx", float3);
            symbolTable.insertBuiltIn(ESSL1_BUILTINS, float4, "dFdx", float4);
            
            symbolTable.insertBuiltIn(ESSL1_BUILTINS, float1, "dFdy", float1);
            symbolTable.insertBuiltIn(ESSL1_BUILTINS, float2, "dFdy", float2);
            symbolTable.insertBuiltIn(ESSL1_BUILTINS, float3, "dFdy", float3);
            symbolTable.insertBuiltIn(ESSL1_BUILTINS, float4, "dFdy", float4);

            symbolTable.insertBuiltIn(ESSL1_BUILTINS, float1, "fwidth", float1);
            symbolTable.insertBuiltIn(ESSL1_BUILTINS, float2, "fwidth", float2);
            symbolTable.insertBuiltIn(ESSL1_BUILTINS, float3, "fwidth", float3);
            symbolTable.insertBuiltIn(ESSL1_BUILTINS, float4, "fwidth", float4);
        }
    }

    if(type == SH_VERTEX_SHADER)
    {
        symbolTable.insertBuiltIn(ESSL1_BUILTINS, float4, "texture2DLod", sampler2D, float2, float1);
        symbolTable.insertBuiltIn(ESSL1_BUILTINS, float4, "texture2DProjLod", sampler2D, float3, float1);
        symbolTable.insertBuiltIn(ESSL1_BUILTINS, float4, "texture2DProjLod", sampler2D, float4, float1);
        symbolTable.insertBuiltIn(ESSL1_BUILTINS, float4, "textureCubeLod", samplerCube, float3, float1);
    }

    TType *sampler3D = new TType(EbtSampler3D, EbpUndefined, EvqGlobal, 1);
    TType *sampler2DArray = new TType(EbtSampler2DArray, EbpUndefined, EvqGlobal, 1);

    //
    // Texture Functions for GLSL ES 3.0
    //
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, float4, "texture", sampler2D, float2);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, float4, "texture", sampler3D, float3);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, float4, "texture", samplerCube, float3);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, float4, "texture", sampler2DArray, float3);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, float4, "textureProj", sampler2D, float3);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, float4, "textureProj", sampler2D, float4);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, float4, "textureProj", sampler3D, float4);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, float4, "textureLod", sampler2D, float2, float1);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, float4, "textureLod", sampler3D, float3, float1);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, float4, "textureLod", samplerCube, float3, float1);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, float4, "textureLod", sampler2DArray, float3, float1);

    if (type == SH_FRAGMENT_SHADER)
    {
        symbolTable.insertBuiltIn(ESSL3_BUILTINS, float4, "texture", sampler2D, float2, float1);
        symbolTable.insertBuiltIn(ESSL3_BUILTINS, float4, "texture", sampler3D, float3, float1);
        symbolTable.insertBuiltIn(ESSL3_BUILTINS, float4, "texture", samplerCube, float3, float1);
        symbolTable.insertBuiltIn(ESSL3_BUILTINS, float4, "texture", sampler2DArray, float3, float1);
        symbolTable.insertBuiltIn(ESSL3_BUILTINS, float4, "textureProj", sampler2D, float3, float1);
        symbolTable.insertBuiltIn(ESSL3_BUILTINS, float4, "textureProj", sampler2D, float4, float1);
        symbolTable.insertBuiltIn(ESSL3_BUILTINS, float4, "textureProj", sampler3D, float4, float1);
    }

    TType *isampler2D = new TType(EbtISampler2D, EbpUndefined, EvqGlobal, 1);
    TType *isampler3D = new TType(EbtISampler3D, EbpUndefined, EvqGlobal, 1);
    TType *isamplerCube = new TType(EbtISamplerCube, EbpUndefined, EvqGlobal, 1);
    TType *isampler2DArray = new TType(EbtISampler2DArray, EbpUndefined, EvqGlobal, 1);

    symbolTable.insertBuiltIn(ESSL3_BUILTINS, int4, "texture", isampler2D, float2);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, int4, "texture", isampler3D, float3);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, int4, "texture", isamplerCube, float3);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, int4, "texture", isampler2DArray, float3);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, int4, "textureProj", isampler2D, float3);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, int4, "textureProj", isampler2D, float4);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, int4, "textureProj", isampler3D, float4);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, int4, "textureLod", isampler2D, float2, float1);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, int4, "textureLod", isampler3D, float3, float1);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, int4, "textureLod", isamplerCube, float3, float1);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, int4, "textureLod", isampler2DArray, float3, float1);

    if (type == SH_FRAGMENT_SHADER)
    {
        symbolTable.insertBuiltIn(ESSL3_BUILTINS, int4, "texture", isampler2D, float2, float1);
        symbolTable.insertBuiltIn(ESSL3_BUILTINS, int4, "texture", isampler3D, float3, float1);
        symbolTable.insertBuiltIn(ESSL3_BUILTINS, int4, "texture", isamplerCube, float3, float1);
        symbolTable.insertBuiltIn(ESSL3_BUILTINS, int4, "texture", isampler2DArray, float3, float1);
        symbolTable.insertBuiltIn(ESSL3_BUILTINS, int4, "textureProj", isampler2D, float3, float1);
        symbolTable.insertBuiltIn(ESSL3_BUILTINS, int4, "textureProj", isampler2D, float4, float1);
        symbolTable.insertBuiltIn(ESSL3_BUILTINS, int4, "textureProj", isampler3D, float4, float1);
    }

    TType *usampler2D = new TType(EbtUSampler2D, EbpUndefined, EvqGlobal, 1);
    TType *usampler3D = new TType(EbtUSampler3D, EbpUndefined, EvqGlobal, 1);
    TType *usamplerCube = new TType(EbtUSamplerCube, EbpUndefined, EvqGlobal, 1);
    TType *usampler2DArray = new TType(EbtUSampler2DArray, EbpUndefined, EvqGlobal, 1);
    TType *uint4 = new TType(EbtUInt, EbpUndefined, EvqGlobal, 4);

    symbolTable.insertBuiltIn(ESSL3_BUILTINS, uint4, "texture", usampler2D, float2);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, uint4, "texture", usampler3D, float3);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, uint4, "texture", usamplerCube, float3);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, uint4, "texture", usampler2DArray, float3);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, uint4, "textureProj", usampler2D, float3);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, uint4, "textureProj", usampler2D, float4);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, uint4, "textureProj", usampler3D, float4);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, uint4, "textureLod", usampler2D, float2, float1);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, uint4, "textureLod", usampler3D, float3, float1);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, uint4, "textureLod", usamplerCube, float3, float1);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, uint4, "textureLod", usampler2DArray, float3, float1);

    if (type == SH_FRAGMENT_SHADER)
    {
        symbolTable.insertBuiltIn(ESSL3_BUILTINS, uint4, "texture", usampler2D, float2, float1);
        symbolTable.insertBuiltIn(ESSL3_BUILTINS, uint4, "texture", usampler3D, float3, float1);
        symbolTable.insertBuiltIn(ESSL3_BUILTINS, uint4, "texture", usamplerCube, float3, float1);
        symbolTable.insertBuiltIn(ESSL3_BUILTINS, uint4, "texture", usampler2DArray, float3, float1);
        symbolTable.insertBuiltIn(ESSL3_BUILTINS, uint4, "textureProj", usampler2D, float3, float1);
        symbolTable.insertBuiltIn(ESSL3_BUILTINS, uint4, "textureProj", usampler2D, float4, float1);
        symbolTable.insertBuiltIn(ESSL3_BUILTINS, uint4, "textureProj", usampler3D, float4, float1);
    }

    TType *sampler2DShadow = new TType(EbtSampler2DShadow, EbpUndefined, EvqGlobal, 1);
    TType *samplerCubeShadow = new TType(EbtSamplerCubeShadow, EbpUndefined, EvqGlobal, 1);
    TType *sampler2DArrayShadow = new TType(EbtSampler2DArrayShadow, EbpUndefined, EvqGlobal, 1);

    symbolTable.insertBuiltIn(ESSL3_BUILTINS, float1, "texture", sampler2DShadow, float3);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, float1, "texture", samplerCubeShadow, float4);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, float1, "texture", sampler2DArrayShadow, float4);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, float1, "textureProj", sampler2DShadow, float4);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, float1, "textureLod", sampler2DShadow, float3, float1);

    if (type == SH_FRAGMENT_SHADER)
    {
        symbolTable.insertBuiltIn(ESSL3_BUILTINS, float1, "texture", sampler2DShadow, float3, float1);
        symbolTable.insertBuiltIn(ESSL3_BUILTINS, float1, "texture", samplerCubeShadow, float4, float1);
        symbolTable.insertBuiltIn(ESSL3_BUILTINS, float1, "texture", sampler2DArrayShadow, float4, float1);
        symbolTable.insertBuiltIn(ESSL3_BUILTINS, float1, "textureProj", sampler2DShadow, float4, float1);
    }

    symbolTable.insertBuiltIn(ESSL3_BUILTINS, int2, "textureSize", sampler2D, int1);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, int2, "textureSize", isampler2D, int1);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, int2, "textureSize", usampler2D, int1);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, int3, "textureSize", sampler3D, int1);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, int3, "textureSize", isampler3D, int1);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, int3, "textureSize", usampler3D, int1);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, int2, "textureSize", samplerCube, int1);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, int2, "textureSize", isamplerCube, int1);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, int2, "textureSize", usamplerCube, int1);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, int3, "textureSize", sampler2DArray, int1);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, int3, "textureSize", isampler2DArray, int1);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, int3, "textureSize", usampler2DArray, int1);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, int2, "textureSize", sampler2DShadow, int1);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, int2, "textureSize", samplerCubeShadow, int1);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, int3, "textureSize", sampler2DArrayShadow, int1);

    if(type == SH_FRAGMENT_SHADER)
    {
        symbolTable.insertBuiltIn(ESSL3_BUILTINS, float1, "dFdx", float1);
        symbolTable.insertBuiltIn(ESSL3_BUILTINS, float2, "dFdx", float2);
        symbolTable.insertBuiltIn(ESSL3_BUILTINS, float3, "dFdx", float3);
        symbolTable.insertBuiltIn(ESSL3_BUILTINS, float4, "dFdx", float4);
            
        symbolTable.insertBuiltIn(ESSL3_BUILTINS, float1, "dFdy", float1);
        symbolTable.insertBuiltIn(ESSL3_BUILTINS, float2, "dFdy", float2);
        symbolTable.insertBuiltIn(ESSL3_BUILTINS, float3, "dFdy", float3);
        symbolTable.insertBuiltIn(ESSL3_BUILTINS, float4, "dFdy", float4);

        symbolTable.insertBuiltIn(ESSL3_BUILTINS, float1, "fwidth", float1);
        symbolTable.insertBuiltIn(ESSL3_BUILTINS, float2, "fwidth", float2);
        symbolTable.insertBuiltIn(ESSL3_BUILTINS, float3, "fwidth", float3);
        symbolTable.insertBuiltIn(ESSL3_BUILTINS, float4, "fwidth", float4);
    }

    symbolTable.insertBuiltIn(ESSL3_BUILTINS, float4, "textureOffset", sampler2D, float2, int2);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, float4, "textureOffset", sampler2D, float2, int2, float1);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, int4, "textureOffset", isampler2D, float2, int2);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, int4, "textureOffset", isampler2D, float2, int2, float1);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, uint4, "textureOffset", usampler2D, float2, int2);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, uint4, "textureOffset", usampler2D, float2, int2, float1);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, float4, "textureOffset", sampler3D, float3, int3);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, float4, "textureOffset", sampler3D, float3, int3, float1);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, int4, "textureOffset", isampler3D, float3, int3);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, int4, "textureOffset", isampler3D, float3, int3, float1);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, uint4, "textureOffset", usampler3D, float3, int3);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, uint4, "textureOffset", usampler3D, float3, int3, float1);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, float1, "textureOffset", sampler2DShadow, float3, int3);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, float1, "textureOffset", sampler2DShadow, float3, int3, float1);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, float4, "textureOffset", sampler2DArray, float3, int2);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, float4, "textureOffset", sampler2DArray, float3, int2, float1);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, int4, "textureOffset", isampler2DArray, float3, int2);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, int4, "textureOffset", isampler2DArray, float3, int2, float1);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, uint4, "textureOffset", usampler2DArray, float3, int2);
    symbolTable.insertBuiltIn(ESSL3_BUILTINS, uint4, "textureOffset", usampler2DArray, float3, int2, float1);

    //
    // Depth range in window coordinates
    //
    TFieldList *fields = NewPoolTFieldList();
    TSourceLoc zeroSourceLoc = {0};
    TField *near = new TField(new TType(EbtFloat, EbpHigh, EvqGlobal, 1), NewPoolTString("near"), zeroSourceLoc);
    TField *far = new TField(new TType(EbtFloat, EbpHigh, EvqGlobal, 1), NewPoolTString("far"), zeroSourceLoc);
    TField *diff = new TField(new TType(EbtFloat, EbpHigh, EvqGlobal, 1), NewPoolTString("diff"), zeroSourceLoc);
    fields->push_back(near);
    fields->push_back(far);
    fields->push_back(diff);
    TStructure *depthRangeStruct = new TStructure(NewPoolTString("gl_DepthRangeParameters"), fields);
    TVariable *depthRangeParameters = new TVariable(&depthRangeStruct->name(), depthRangeStruct, true);
    symbolTable.insert(COMMON_BUILTINS, *depthRangeParameters);
    TVariable *depthRange = new TVariable(NewPoolTString("gl_DepthRange"), TType(depthRangeStruct));
    depthRange->setQualifier(EvqUniform);
    symbolTable.insert(COMMON_BUILTINS, *depthRange);

    //
    // Implementation dependent built-in constants.
    //
    symbolTable.insertConstInt(COMMON_BUILTINS, "gl_MaxVertexAttribs", resources.MaxVertexAttribs);
    symbolTable.insertConstInt(COMMON_BUILTINS, "gl_MaxVertexUniformVectors", resources.MaxVertexUniformVectors);
    symbolTable.insertConstInt(COMMON_BUILTINS, "gl_MaxVertexTextureImageUnits", resources.MaxVertexTextureImageUnits);
    symbolTable.insertConstInt(COMMON_BUILTINS, "gl_MaxCombinedTextureImageUnits", resources.MaxCombinedTextureImageUnits);
    symbolTable.insertConstInt(COMMON_BUILTINS, "gl_MaxTextureImageUnits", resources.MaxTextureImageUnits);
    symbolTable.insertConstInt(COMMON_BUILTINS, "gl_MaxFragmentUniformVectors", resources.MaxFragmentUniformVectors);

    symbolTable.insertConstInt(ESSL1_BUILTINS, "gl_MaxVaryingVectors", resources.MaxVaryingVectors);

    if (spec != SH_CSS_SHADERS_SPEC)
    {
        symbolTable.insertConstInt(COMMON_BUILTINS, "gl_MaxDrawBuffers", resources.MaxDrawBuffers);
    }

    symbolTable.insertConstInt(ESSL3_BUILTINS, "gl_MaxVertexOutputVectors", resources.MaxVertexOutputVectors);
    symbolTable.insertConstInt(ESSL3_BUILTINS, "gl_MaxFragmentInputVectors", resources.MaxFragmentInputVectors);
    symbolTable.insertConstInt(ESSL3_BUILTINS, "gl_MinProgramTexelOffset", resources.MinProgramTexelOffset);
    symbolTable.insertConstInt(ESSL3_BUILTINS, "gl_MaxProgramTexelOffset", resources.MaxProgramTexelOffset);
}

void IdentifyBuiltIns(ShShaderType type, ShShaderSpec spec,
                      const ShBuiltInResources &resources,
                      TSymbolTable &symbolTable)
{
    //
    // First, insert some special built-in variables that are not in 
    // the built-in header files.
    //
    switch(type) {
    case SH_FRAGMENT_SHADER:
        symbolTable.insert(COMMON_BUILTINS, *new TVariable(NewPoolTString("gl_FragCoord"), TType(EbtFloat, EbpMedium, EvqFragCoord,   4)));
        symbolTable.insert(COMMON_BUILTINS, *new TVariable(NewPoolTString("gl_FrontFacing"), TType(EbtBool,  EbpUndefined, EvqFrontFacing, 1)));
        symbolTable.insert(COMMON_BUILTINS, *new TVariable(NewPoolTString("gl_PointCoord"), TType(EbtFloat, EbpMedium, EvqPointCoord,  2)));

        //
        // In CSS Shaders, gl_FragColor, gl_FragData, and gl_MaxDrawBuffers are not available.
        // Instead, css_MixColor and css_ColorMatrix are available.
        //
        if (spec != SH_CSS_SHADERS_SPEC) {
            symbolTable.insert(ESSL1_BUILTINS, *new TVariable(NewPoolTString("gl_FragColor"), TType(EbtFloat, EbpMedium, EvqFragColor,   4)));
            symbolTable.insert(ESSL1_BUILTINS, *new TVariable(NewPoolTString("gl_FragData[gl_MaxDrawBuffers]"), TType(EbtFloat, EbpMedium, EvqFragData,    4)));
            if (resources.EXT_frag_depth) {
                symbolTable.insert(ESSL1_BUILTINS, *new TVariable(NewPoolTString("gl_FragDepthEXT"), TType(EbtFloat, resources.FragmentPrecisionHigh ? EbpHigh : EbpMedium, EvqFragDepth, 1)));
                symbolTable.relateToExtension(ESSL1_BUILTINS, "gl_FragDepthEXT", "GL_EXT_frag_depth");
            }
        } else {
            symbolTable.insert(ESSL1_BUILTINS, *new TVariable(NewPoolTString("css_MixColor"), TType(EbtFloat, EbpMedium, EvqGlobal,      4)));
            symbolTable.insert(ESSL1_BUILTINS, *new TVariable(NewPoolTString("css_ColorMatrix"), TType(EbtFloat, EbpMedium, EvqGlobal,      4, 4)));
        }

        break;

    case SH_VERTEX_SHADER:
        symbolTable.insert(COMMON_BUILTINS, *new TVariable(NewPoolTString("gl_Position"), TType(EbtFloat, EbpHigh, EvqPosition,    4)));
        symbolTable.insert(COMMON_BUILTINS, *new TVariable(NewPoolTString("gl_PointSize"), TType(EbtFloat, EbpMedium, EvqPointSize,   1)));
        break;

    default: assert(false && "Language not supported");
    }

    //
    // Next, identify which built-ins from the already loaded headers have
    // a mapping to an operator.  Those that are not identified as such are
    // expected to be resolved through a library of functions, versus as
    // operations.
    //
    symbolTable.relateToOperator(COMMON_BUILTINS, "matrixCompMult",   EOpMul);

    symbolTable.relateToOperator(COMMON_BUILTINS, "equal",            EOpVectorEqual);
    symbolTable.relateToOperator(COMMON_BUILTINS, "notEqual",         EOpVectorNotEqual);
    symbolTable.relateToOperator(COMMON_BUILTINS, "lessThan",         EOpLessThan);
    symbolTable.relateToOperator(COMMON_BUILTINS, "greaterThan",      EOpGreaterThan);
    symbolTable.relateToOperator(COMMON_BUILTINS, "lessThanEqual",    EOpLessThanEqual);
    symbolTable.relateToOperator(COMMON_BUILTINS, "greaterThanEqual", EOpGreaterThanEqual);
    
    symbolTable.relateToOperator(COMMON_BUILTINS, "radians",      EOpRadians);
    symbolTable.relateToOperator(COMMON_BUILTINS, "degrees",      EOpDegrees);
    symbolTable.relateToOperator(COMMON_BUILTINS, "sin",          EOpSin);
    symbolTable.relateToOperator(COMMON_BUILTINS, "cos",          EOpCos);
    symbolTable.relateToOperator(COMMON_BUILTINS, "tan",          EOpTan);
    symbolTable.relateToOperator(COMMON_BUILTINS, "asin",         EOpAsin);
    symbolTable.relateToOperator(COMMON_BUILTINS, "acos",         EOpAcos);
    symbolTable.relateToOperator(COMMON_BUILTINS, "atan",         EOpAtan);

    symbolTable.relateToOperator(COMMON_BUILTINS, "pow",          EOpPow);
    symbolTable.relateToOperator(COMMON_BUILTINS, "exp2",         EOpExp2);
    symbolTable.relateToOperator(COMMON_BUILTINS, "log",          EOpLog);
    symbolTable.relateToOperator(COMMON_BUILTINS, "exp",          EOpExp);
    symbolTable.relateToOperator(COMMON_BUILTINS, "log2",         EOpLog2);
    symbolTable.relateToOperator(COMMON_BUILTINS, "sqrt",         EOpSqrt);
    symbolTable.relateToOperator(COMMON_BUILTINS, "inversesqrt",  EOpInverseSqrt);

    symbolTable.relateToOperator(COMMON_BUILTINS, "abs",          EOpAbs);
    symbolTable.relateToOperator(COMMON_BUILTINS, "sign",         EOpSign);
    symbolTable.relateToOperator(COMMON_BUILTINS, "floor",        EOpFloor);
    symbolTable.relateToOperator(COMMON_BUILTINS, "ceil",         EOpCeil);
    symbolTable.relateToOperator(COMMON_BUILTINS, "fract",        EOpFract);
    symbolTable.relateToOperator(COMMON_BUILTINS, "mod",          EOpMod);
    symbolTable.relateToOperator(COMMON_BUILTINS, "min",          EOpMin);
    symbolTable.relateToOperator(COMMON_BUILTINS, "max",          EOpMax);
    symbolTable.relateToOperator(COMMON_BUILTINS, "clamp",        EOpClamp);
    symbolTable.relateToOperator(COMMON_BUILTINS, "mix",          EOpMix);
    symbolTable.relateToOperator(COMMON_BUILTINS, "step",         EOpStep);
    symbolTable.relateToOperator(COMMON_BUILTINS, "smoothstep",   EOpSmoothStep);

    symbolTable.relateToOperator(COMMON_BUILTINS, "length",       EOpLength);
    symbolTable.relateToOperator(COMMON_BUILTINS, "distance",     EOpDistance);
    symbolTable.relateToOperator(COMMON_BUILTINS, "dot",          EOpDot);
    symbolTable.relateToOperator(COMMON_BUILTINS, "cross",        EOpCross);
    symbolTable.relateToOperator(COMMON_BUILTINS, "normalize",    EOpNormalize);
    symbolTable.relateToOperator(COMMON_BUILTINS, "faceforward",  EOpFaceForward);
    symbolTable.relateToOperator(COMMON_BUILTINS, "reflect",      EOpReflect);
    symbolTable.relateToOperator(COMMON_BUILTINS, "refract",      EOpRefract);
    
    symbolTable.relateToOperator(COMMON_BUILTINS, "any",          EOpAny);
    symbolTable.relateToOperator(COMMON_BUILTINS, "all",          EOpAll);
    symbolTable.relateToOperator(COMMON_BUILTINS, "not",          EOpVectorLogicalNot);

    // Map language-specific operators.
    switch(type) {
    case SH_VERTEX_SHADER:
        break;
    case SH_FRAGMENT_SHADER:
        if (resources.OES_standard_derivatives) {
            symbolTable.relateToOperator(ESSL1_BUILTINS, "dFdx",   EOpDFdx);
            symbolTable.relateToOperator(ESSL1_BUILTINS, "dFdy",   EOpDFdy);
            symbolTable.relateToOperator(ESSL1_BUILTINS, "fwidth", EOpFwidth);

            symbolTable.relateToExtension(ESSL1_BUILTINS, "dFdx", "GL_OES_standard_derivatives");
            symbolTable.relateToExtension(ESSL1_BUILTINS, "dFdy", "GL_OES_standard_derivatives");
            symbolTable.relateToExtension(ESSL1_BUILTINS, "fwidth", "GL_OES_standard_derivatives");
        }
        break;
    default: break;
    }

    symbolTable.relateToOperator(ESSL3_BUILTINS, "dFdx",   EOpDFdx);
    symbolTable.relateToOperator(ESSL3_BUILTINS, "dFdy",   EOpDFdy);
    symbolTable.relateToOperator(ESSL3_BUILTINS, "fwidth", EOpFwidth);

    // Finally add resource-specific variables.
    switch(type) {
    case SH_FRAGMENT_SHADER:
        if (spec != SH_CSS_SHADERS_SPEC) {
            // Set up gl_FragData.  The array size.
            TType fragData(EbtFloat, EbpMedium, EvqFragData, 4, 1, true);
            fragData.setArraySize(resources.MaxDrawBuffers);
            symbolTable.insert(ESSL1_BUILTINS, *new TVariable(NewPoolTString("gl_FragData"), fragData));
        }
        break;
    default: break;
    }
}

void InitExtensionBehavior(const ShBuiltInResources& resources,
                           TExtensionBehavior& extBehavior)
{
    if (resources.OES_standard_derivatives)
        extBehavior["GL_OES_standard_derivatives"] = EBhUndefined;
    if (resources.OES_EGL_image_external)
        extBehavior["GL_OES_EGL_image_external"] = EBhUndefined;
    if (resources.ARB_texture_rectangle)
        extBehavior["GL_ARB_texture_rectangle"] = EBhUndefined;
    if (resources.EXT_draw_buffers)
        extBehavior["GL_EXT_draw_buffers"] = EBhUndefined;
    if (resources.EXT_frag_depth)
        extBehavior["GL_EXT_frag_depth"] = EBhUndefined;
}
