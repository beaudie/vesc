//
// Copyright (c) 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// UtilsHLSL.h:
//   Utility methods for GLSL to HLSL translation.
//

#ifndef COMPILER_TRANSLATOR_UTILSHLSL_H_
#define COMPILER_TRANSLATOR_UTILSHLSL_H_

#include <vector>
#include "compiler/translator/IntermNode.h"
#include "compiler/translator/Types.h"

#include "angle_gl.h"

class TName;

namespace sh
{

// Unique combinations of HLSL Texture type and HLSL Sampler type.
enum HLSLTextureSamplerGroup
{
    // Regular samplers
    HLSL_TEXTURE_2D_SAMPLER,
    HLSL_TEXTURE_SAMPLER_MIN = HLSL_TEXTURE_2D_SAMPLER,

    HLSL_TEXTURE_CUBE_SAMPLER,
    HLSL_TEXTURE_2D_ARRAY_SAMPLER,
    HLSL_TEXTURE_3D_SAMPLER,
    HLSL_TEXTURE_2D_MS_SAMPLER,
    HLSL_TEXTURE_2D_INT4_SAMPLER,
    HLSL_TEXTURE_3D_INT4_SAMPLER,
    HLSL_TEXTURE_2D_ARRAY_INT4_SAMPLER,
    HLSL_TEXTURE_2D_MS_INT4_SAMPLER,
    HLSL_TEXTURE_2D_UINT4_SAMPLER,
    HLSL_TEXTURE_3D_UINT4_SAMPLER,
    HLSL_TEXTURE_2D_ARRAY_UINT4_SAMPLER,
    HLSL_TEXTURE_2D_MS_UINT4_SAMPLER,

    // Comparison samplers

    HLSL_TEXTURE_2D_COMPARISON_SAMPLER,
    HLSL_TEXTURE_CUBE_COMPARISON_SAMPLER,
    HLSL_TEXTURE_2D_ARRAY_COMPARISON_SAMPLER,

    HLSL_COMPARISON_SAMPLER_GROUP_BEGIN = HLSL_TEXTURE_2D_COMPARISON_SAMPLER,
    HLSL_COMPARISON_SAMPLER_GROUP_END   = HLSL_TEXTURE_2D_ARRAY_COMPARISON_SAMPLER,

    HLSL_TEXTURE_SAMPLER_UNKNOWN,
    HLSL_TEXTURE_SAMPLER_MAX = HLSL_TEXTURE_SAMPLER_UNKNOWN
};

// Unique combinations for HLSL Texture type and GLSL image type.
enum HLSLTextureGroup
{
    HLSL_TEXTURE_2D_FLOAT4,
    HLSL_TEXTURE_MIN = HLSL_TEXTURE_2D_FLOAT4,
    HLSL_TEXTURE_2D_ARRAY_FLOAT4,
    HLSL_TEXTURE_3D_FLOAT4,
    HLSL_TEXTURE_2D_UINT4,
    HLSL_TEXTURE_2D_ARRAY_UINT4,
    HLSL_TEXTURE_3D_UINT4,
    HLSL_TEXTURE_2D_INT4,
    HLSL_TEXTURE_2D_ARRAY_INT4,
    HLSL_TEXTURE_3D_INT4,

    HLSL_RWTEXTURE_2D_FLOAT4,
    HLSL_RWTEXTURE_2D_ARRAY_FLOAT4,
    HLSL_RWTEXTURE_3D_FLOAT4,
    HLSL_RWTEXTURE_2D_UINT4,
    HLSL_RWTEXTURE_2D_ARRAY_UINT4,
    HLSL_RWTEXTURE_3D_UINT4,
    HLSL_RWTEXTURE_2D_INT4,
    HLSL_RWTEXTURE_2D_ARRAY_INT4,
    HLSL_RWTEXTURE_3D_INT4,

    HLSL_TEXTURE_SRV_GROUP_BEGIN = HLSL_TEXTURE_2D_FLOAT4,
    HLSL_TEXTURE_SRV_GROUP_END   = HLSL_TEXTURE_3D_INT4,
    HLSL_TEXTURE_UAV_GROUP_BEGIN = HLSL_RWTEXTURE_2D_FLOAT4,
    HLSL_TEXTURE_UAV_GROUP_END   = HLSL_RWTEXTURE_3D_INT4,

    HLSL_TEXTURE_UNKNOWN,
    HLSL_TEXTURE_MAX = HLSL_TEXTURE_UNKNOWN
};

HLSLTextureSamplerGroup TextureSamplerGroup(const TBasicType type);
TString TextureSamplerString(const HLSLTextureSamplerGroup type);
TString TextureSamplerString(const TBasicType type);
TString TextureSamplerGroupSuffix(const HLSLTextureSamplerGroup type);
TString TextureSamplerGroupSuffix(const TBasicType type);
TString TextureSamplerTypeSuffix(const TBasicType type);
TString SamplerString(const TBasicType type);
TString SamplerString(HLSLTextureSamplerGroup type);

HLSLTextureGroup TextureGroup(const TBasicType type,
                              TLayoutImageInternalFormat imageInternalFormat,
                              bool readonly);
TString TextureGroupSuffix(const HLSLTextureGroup type);
TString TextureTypeSuffix(const TBasicType type,
                          TLayoutImageInternalFormat imageInternalFormat,
                          bool readonly);
TString TextureString(const HLSLTextureGroup textureGroup);
// Adds a prefix to user-defined names to avoid naming clashes.
TString Decorate(const TString &string);
TString DecorateVariableIfNeeded(const TName &name);
TString DecorateFunctionIfNeeded(const TName &name);
TString DecorateField(const TString &string, const TStructure &structure);
TString DecoratePrivate(const TString &privateText);
TString TypeString(const TType &type);
TString StructNameString(const TStructure &structure);
TString QualifiedStructNameString(const TStructure &structure,
                                  bool useHLSLRowMajorPacking,
                                  bool useStd140Packing);
TString InterpolationString(TQualifier qualifier);
TString QualifierString(TQualifier qualifier);
// Parameters may need to be included in function names to disambiguate between overloaded
// functions.
TString DisambiguateFunctionName(const TIntermSequence *parameters);
}

#endif  // COMPILER_TRANSLATOR_UTILSHLSL_H_
