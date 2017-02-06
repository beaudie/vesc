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
    HLSL_TEXTURE_2D,
    HLSL_TEXTURE_MIN = HLSL_TEXTURE_2D,

    HLSL_TEXTURE_CUBE,
    HLSL_TEXTURE_2D_ARRAY,
    HLSL_TEXTURE_3D,
    HLSL_TEXTURE_2D_MS,
    HLSL_TEXTURE_2D_INT4,
    HLSL_TEXTURE_3D_INT4,
    HLSL_TEXTURE_2D_ARRAY_INT4,
    HLSL_TEXTURE_2D_MS_INT4,
    HLSL_TEXTURE_2D_UINT4,
    HLSL_TEXTURE_3D_UINT4,
    HLSL_TEXTURE_2D_ARRAY_UINT4,
    HLSL_TEXTURE_2D_MS_UINT4,

    // Comparison samplers

    HLSL_TEXTURE_2D_COMPARISON,
    HLSL_TEXTURE_CUBE_COMPARISON,
    HLSL_TEXTURE_2D_ARRAY_COMPARISON,

    HLSL_COMPARISON_SAMPLER_GROUP_BEGIN = HLSL_TEXTURE_2D_COMPARISON,
    HLSL_COMPARISON_SAMPLER_GROUP_END   = HLSL_TEXTURE_2D_ARRAY_COMPARISON,

    HLSL_TEXTURE_UNKNOWN,
    HLSL_TEXTURE_MAX = HLSL_TEXTURE_UNKNOWN
};

// Unique combinations for HLSL Shader Resource View type and HLSL Texture type.
enum HLSLSRVTextureGroup
{
    HLSL_SRV_TEXTURE_2D_FLOAT4,
    HLSL_SRV_TEXTURE_MIN = HLSL_SRV_TEXTURE_2D_FLOAT4,
    HLSL_SRV_TEXTURE_2D_ARRAY_FLOAT4,
    HLSL_SRV_TEXTURE_CUBE_FLOAT4,
    HLSL_SRV_TEXTURE_3D_FLOAT4,
    HLSL_SRV_TEXTURE_2D_FLOAT,
    HLSL_SRV_TEXTURE_2D_ARRAY_FLOAT,
    HLSL_SRV_TEXTURE_CUBE_FLOAT,
    HLSL_SRV_TEXTURE_3D_FLOAT,
    HLSL_SRV_TEXTURE_2D_UINT4,
    HLSL_SRV_TEXTURE_2D_ARRAY_UINT4,
    HLSL_SRV_TEXTURE_CUBE_UINT4,
    HLSL_SRV_TEXTURE_3D_UINT4,
    HLSL_SRV_TEXTURE_2D_UINT,
    HLSL_SRV_TEXTURE_2D_ARRAY_UINT,
    HLSL_SRV_TEXTURE_CUBE_UINT,
    HLSL_SRV_TEXTURE_3D_UINT,
    HLSL_SRV_TEXTURE_2D_INT4,
    HLSL_SRV_TEXTURE_2D_ARRAY_INT4,
    HLSL_SRV_TEXTURE_CUBE_INT4,
    HLSL_SRV_TEXTURE_3D_INT4,
    HLSL_SRV_TEXTURE_2D_INT,
    HLSL_SRV_TEXTURE_2D_ARRAY_INT,
    HLSL_SRV_TEXTURE_CUBE_INT,
    HLSL_SRV_TEXTURE_3D_INT,

    HLSL_SRV_TEXTURE_UNKNOWN,
    HLSL_SRV_TEXTURE_MAX = HLSL_SRV_TEXTURE_UNKNOWN
};

// Unique combinations for HLSL Unordered Access View type and HLSL Texture type.
enum HLSLUAVTextureGroup
{
    HLSL_UAV_TEXTURE_2D_FLOAT4,
    HLSL_UAV_TEXTURE_MIN = HLSL_UAV_TEXTURE_2D_FLOAT4,
    HLSL_UAV_TEXTURE_2D_ARRAY_FLOAT4,
    HLSL_UAV_TEXTURE_3D_FLOAT4,
    HLSL_UAV_TEXTURE_2D_FLOAT,
    HLSL_UAV_TEXTURE_2D_ARRAY_FLOAT,
    HLSL_UAV_TEXTURE_3D_FLOAT,
    HLSL_UAV_TEXTURE_2D_UINT4,
    HLSL_UAV_TEXTURE_2D_ARRAY_UINT4,
    HLSL_UAV_TEXTURE_3D_UINT4,
    HLSL_UAV_TEXTURE_2D_UINT,
    HLSL_UAV_TEXTURE_2D_ARRAY_UINT,
    HLSL_UAV_TEXTURE_3D_UINT,
    HLSL_UAV_TEXTURE_2D_INT4,
    HLSL_UAV_TEXTURE_2D_ARRAY_INT4,
    HLSL_UAV_TEXTURE_3D_INT4,
    HLSL_UAV_TEXTURE_2D_INT,
    HLSL_UAV_TEXTURE_2D_ARRAY_INT,
    HLSL_UAV_TEXTURE_3D_INT,

    HLSL_UAV_TEXTURE_UNKNOWN,
    HLSL_UAV_TEXTURE_MAX = HLSL_UAV_TEXTURE_UNKNOWN
};

HLSLTextureSamplerGroup TextureGroup(const TBasicType type);
TString TextureString(const HLSLTextureSamplerGroup type);
TString TextureString(const TBasicType type);
TString TextureGroupSuffix(const HLSLTextureSamplerGroup type);
TString TextureGroupSuffix(const TBasicType type);
TString TextureTypeSuffix(const TBasicType type);
TString SamplerString(const TBasicType type);
TString SamplerString(HLSLTextureSamplerGroup type);
HLSLSRVTextureGroup SRVTextureGroup(const TBasicType type,
                                    TLayoutImageInternalFormat imageInternalFormat);
HLSLUAVTextureGroup UAVTextureGroup(const TBasicType type,
                                    TLayoutImageInternalFormat imageInternalFormat);
TString SRVTextureGroupSuffix(const HLSLSRVTextureGroup type);
TString UAVTextureGroupSuffix(const HLSLUAVTextureGroup type);
TString SRVTextureString(const HLSLSRVTextureGroup SRVTextureGroup);
TString UAVTextureString(const HLSLUAVTextureGroup UAVTextureGroup);

// Prepends an underscore to avoid naming clashes
TString Decorate(const TString &string);
TString DecorateIfNeeded(const TName &name);
TString DecorateUniform(const TName &name, const TType &type);
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
