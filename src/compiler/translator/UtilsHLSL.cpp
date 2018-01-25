//
// Copyright (c) 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// UtilsHLSL.cpp:
//   Utility methods for GLSL to HLSL translation.
//

#include "compiler/translator/UtilsHLSL.h"
#include "compiler/translator/ImmutableStringBuilder.h"
#include "compiler/translator/IntermNode.h"
#include "compiler/translator/StructureHLSL.h"
#include "compiler/translator/SymbolTable.h"

namespace sh
{

namespace
{

constexpr const ImmutableString kFloatTypeNames[5] = {
    ImmutableString(""), ImmutableString("float"), ImmutableString("float2"),
    ImmutableString("float3"), ImmutableString("float4")};
constexpr const ImmutableString kIntTypeNames[5] = {
    ImmutableString(""), ImmutableString("int"), ImmutableString("int2"), ImmutableString("int3"),
    ImmutableString("int4")};
constexpr const ImmutableString kUIntTypeNames[5] = {
    ImmutableString(""), ImmutableString("uint"), ImmutableString("uint2"),
    ImmutableString("uint3"), ImmutableString("uint4")};
constexpr const ImmutableString kBoolTypeNames[5] = {
    ImmutableString(""), ImmutableString("bool"), ImmutableString("bool2"),
    ImmutableString("bool3"), ImmutableString("bool4")};

void DisambiguateFunctionNameForParameterType(const TType &paramType,
                                              TStringStream *disambiguatingStringOut)
{
    // Parameter types are only added to function names if they are ambiguous according to the
    // native HLSL compiler. Other parameter types are not added to function names to avoid
    // making function names longer.
    if (paramType.getObjectSize() == 4 && paramType.getBasicType() == EbtFloat)
    {
        // Disambiguation is needed for float2x2 and float4 parameters. These are the only
        // built-in types that HLSL thinks are identical. float2x3 and float3x2 are different
        // types, for example.
        *disambiguatingStringOut << "_" << TypeString(paramType);
    }
    else if (paramType.getBasicType() == EbtStruct)
    {
        // Disambiguation is needed for struct parameters, since HLSL thinks that structs with
        // the same fields but a different name are identical.
        ASSERT(paramType.getStruct()->symbolType() != SymbolType::Empty);
        *disambiguatingStringOut << "_" << TypeString(paramType);
    }
}

}  // anonymous namespace

const char *SamplerString(const TBasicType type)
{
    if (IsShadowSampler(type))
    {
        return "SamplerComparisonState";
    }
    else
    {
        return "SamplerState";
    }
}

const char *SamplerString(HLSLTextureGroup type)
{
    if (type >= HLSL_COMPARISON_SAMPLER_GROUP_BEGIN && type <= HLSL_COMPARISON_SAMPLER_GROUP_END)
    {
        return "SamplerComparisonState";
    }
    else
    {
        return "SamplerState";
    }
}

HLSLTextureGroup TextureGroup(const TBasicType type, TLayoutImageInternalFormat imageInternalFormat)

{
    switch (type)
    {
        case EbtSampler2D:
            return HLSL_TEXTURE_2D;
        case EbtSamplerCube:
            return HLSL_TEXTURE_CUBE;
        case EbtSamplerExternalOES:
            return HLSL_TEXTURE_2D;
        case EbtSampler2DArray:
            return HLSL_TEXTURE_2D_ARRAY;
        case EbtSampler3D:
            return HLSL_TEXTURE_3D;
        case EbtSampler2DMS:
            return HLSL_TEXTURE_2D_MS;
        case EbtISampler2D:
            return HLSL_TEXTURE_2D_INT4;
        case EbtISampler3D:
            return HLSL_TEXTURE_3D_INT4;
        case EbtISamplerCube:
            return HLSL_TEXTURE_2D_ARRAY_INT4;
        case EbtISampler2DArray:
            return HLSL_TEXTURE_2D_ARRAY_INT4;
        case EbtISampler2DMS:
            return HLSL_TEXTURE_2D_MS_INT4;
        case EbtUSampler2D:
            return HLSL_TEXTURE_2D_UINT4;
        case EbtUSampler3D:
            return HLSL_TEXTURE_3D_UINT4;
        case EbtUSamplerCube:
            return HLSL_TEXTURE_2D_ARRAY_UINT4;
        case EbtUSampler2DArray:
            return HLSL_TEXTURE_2D_ARRAY_UINT4;
        case EbtUSampler2DMS:
            return HLSL_TEXTURE_2D_MS_UINT4;
        case EbtSampler2DShadow:
            return HLSL_TEXTURE_2D_COMPARISON;
        case EbtSamplerCubeShadow:
            return HLSL_TEXTURE_CUBE_COMPARISON;
        case EbtSampler2DArrayShadow:
            return HLSL_TEXTURE_2D_ARRAY_COMPARISON;
        case EbtImage2D:
        {
            switch (imageInternalFormat)
            {
                case EiifRGBA32F:
                case EiifRGBA16F:
                case EiifR32F:
                    return HLSL_TEXTURE_2D;
                case EiifRGBA8:
                    return HLSL_TEXTURE_2D_UNORM;
                case EiifRGBA8_SNORM:
                    return HLSL_TEXTURE_2D_SNORM;
                default:
                    UNREACHABLE();
#if !UNREACHABLE_IS_NORETURN
                    return HLSL_TEXTURE_UNKNOWN;
#endif
            }
        }
        case EbtIImage2D:
        {
            switch (imageInternalFormat)
            {
                case EiifRGBA32I:
                case EiifRGBA16I:
                case EiifRGBA8I:
                case EiifR32I:
                    return HLSL_TEXTURE_2D_INT4;
                default:
                    UNREACHABLE();
#if !UNREACHABLE_IS_NORETURN
                    return HLSL_TEXTURE_UNKNOWN;
#endif
            }
        }
        case EbtUImage2D:
        {
            switch (imageInternalFormat)
            {

                case EiifRGBA32UI:
                case EiifRGBA16UI:
                case EiifRGBA8UI:
                case EiifR32UI:
                    return HLSL_TEXTURE_2D_UINT4;
                default:
                    UNREACHABLE();
#if !UNREACHABLE_IS_NORETURN
                    return HLSL_TEXTURE_UNKNOWN;
#endif
            }
        }
        case EbtImage3D:
        {
            switch (imageInternalFormat)
            {
                case EiifRGBA32F:
                case EiifRGBA16F:
                case EiifR32F:
                    return HLSL_TEXTURE_3D;
                case EiifRGBA8:
                    return HLSL_TEXTURE_3D_UNORM;
                case EiifRGBA8_SNORM:
                    return HLSL_TEXTURE_3D_SNORM;
                default:
                    UNREACHABLE();
#if !UNREACHABLE_IS_NORETURN
                    return HLSL_TEXTURE_UNKNOWN;
#endif
            }
        }
        case EbtIImage3D:
        {
            switch (imageInternalFormat)
            {
                case EiifRGBA32I:
                case EiifRGBA16I:
                case EiifRGBA8I:
                case EiifR32I:
                    return HLSL_TEXTURE_3D_INT4;
                default:
                    UNREACHABLE();
#if !UNREACHABLE_IS_NORETURN
                    return HLSL_TEXTURE_UNKNOWN;
#endif
            }
        }
        case EbtUImage3D:
        {
            switch (imageInternalFormat)
            {
                case EiifRGBA32UI:
                case EiifRGBA16UI:
                case EiifRGBA8UI:
                case EiifR32UI:
                    return HLSL_TEXTURE_3D_UINT4;
                default:
                    UNREACHABLE();
#if !UNREACHABLE_IS_NORETURN
                    return HLSL_TEXTURE_UNKNOWN;
#endif
            }
        }
        case EbtImage2DArray:
        case EbtImageCube:
        {
            switch (imageInternalFormat)
            {
                case EiifRGBA32F:
                case EiifRGBA16F:
                case EiifR32F:
                    return HLSL_TEXTURE_2D_ARRAY;
                case EiifRGBA8:
                    return HLSL_TEXTURE_2D_ARRAY_UNORN;
                case EiifRGBA8_SNORM:
                    return HLSL_TEXTURE_2D_ARRAY_SNORM;
                default:
                    UNREACHABLE();
#if !UNREACHABLE_IS_NORETURN
                    return HLSL_TEXTURE_UNKNOWN;
#endif
            }
        }
        case EbtIImage2DArray:
        case EbtIImageCube:
        {
            switch (imageInternalFormat)
            {
                case EiifRGBA32I:
                case EiifRGBA16I:
                case EiifRGBA8I:
                case EiifR32I:
                    return HLSL_TEXTURE_2D_ARRAY_INT4;
                default:
                    UNREACHABLE();
#if !UNREACHABLE_IS_NORETURN
                    return HLSL_TEXTURE_UNKNOWN;
#endif
            }
        }
        case EbtUImage2DArray:
        case EbtUImageCube:
        {
            switch (imageInternalFormat)
            {
                case EiifRGBA32UI:
                case EiifRGBA16UI:
                case EiifRGBA8UI:
                case EiifR32UI:
                    return HLSL_TEXTURE_2D_ARRAY_UINT4;
                default:
                    UNREACHABLE();
#if !UNREACHABLE_IS_NORETURN
                    return HLSL_TEXTURE_UNKNOWN;
#endif
            }
        }
        default:
            UNREACHABLE();
#if !UNREACHABLE_IS_NORETURN
            return HLSL_TEXTURE_UNKNOWN;
#endif
    }
}

const char *TextureString(const HLSLTextureGroup textureGroup)
{
    switch (textureGroup)
    {
        case HLSL_TEXTURE_2D:
            return "Texture2D<float4>";
        case HLSL_TEXTURE_CUBE:
            return "TextureCube<float4>";
        case HLSL_TEXTURE_2D_ARRAY:
            return "Texture2DArray<float4>";
        case HLSL_TEXTURE_3D:
            return "Texture3D<float4>";
        case HLSL_TEXTURE_2D_UNORM:
            return "Texture2D<unorm float4>";
        case HLSL_TEXTURE_CUBE_UNORM:
            return "TextureCube<unorm float4>";
        case HLSL_TEXTURE_2D_ARRAY_UNORN:
            return "Texture2DArray<unorm float4>";
        case HLSL_TEXTURE_3D_UNORM:
            return "Texture3D<unorm float4>";
        case HLSL_TEXTURE_2D_SNORM:
            return "Texture2D<snorm float4>";
        case HLSL_TEXTURE_CUBE_SNORM:
            return "TextureCube<snorm float4>";
        case HLSL_TEXTURE_2D_ARRAY_SNORM:
            return "Texture2DArray<snorm float4>";
        case HLSL_TEXTURE_3D_SNORM:
            return "Texture3D<snorm float4>";
        case HLSL_TEXTURE_2D_MS:
            return "Texture2DMS<float4>";
        case HLSL_TEXTURE_2D_INT4:
            return "Texture2D<int4>";
        case HLSL_TEXTURE_3D_INT4:
            return "Texture3D<int4>";
        case HLSL_TEXTURE_2D_ARRAY_INT4:
            return "Texture2DArray<int4>";
        case HLSL_TEXTURE_2D_MS_INT4:
            return "Texture2DMS<int4>";
        case HLSL_TEXTURE_2D_UINT4:
            return "Texture2D<uint4>";
        case HLSL_TEXTURE_3D_UINT4:
            return "Texture3D<uint4>";
        case HLSL_TEXTURE_2D_ARRAY_UINT4:
            return "Texture2DArray<uint4>";
        case HLSL_TEXTURE_2D_MS_UINT4:
            return "Texture2DMS<uint4>";
        case HLSL_TEXTURE_2D_COMPARISON:
            return "Texture2D";
        case HLSL_TEXTURE_CUBE_COMPARISON:
            return "TextureCube";
        case HLSL_TEXTURE_2D_ARRAY_COMPARISON:
            return "Texture2DArray";
        default:
            UNREACHABLE();
    }

    return "<unknown read texture type>";
}

const char *TextureString(const TBasicType type, TLayoutImageInternalFormat imageInternalFormat)
{
    return TextureString(TextureGroup(type, imageInternalFormat));
}

const char *TextureGroupSuffix(const HLSLTextureGroup type)
{
    switch (type)
    {
        case HLSL_TEXTURE_2D:
            return "2D";
        case HLSL_TEXTURE_CUBE:
            return "Cube";
        case HLSL_TEXTURE_2D_ARRAY:
            return "2DArray";
        case HLSL_TEXTURE_3D:
            return "3D";
        case HLSL_TEXTURE_2D_UNORM:
            return "2D_unorm_float4_";
        case HLSL_TEXTURE_CUBE_UNORM:
            return "Cube_unorm_float4_";
        case HLSL_TEXTURE_2D_ARRAY_UNORN:
            return "2DArray_unorm_float4_";
        case HLSL_TEXTURE_3D_UNORM:
            return "3D_unorm_float4_";
        case HLSL_TEXTURE_2D_SNORM:
            return "2D_snorm_float4_";
        case HLSL_TEXTURE_CUBE_SNORM:
            return "Cube_snorm_float4_";
        case HLSL_TEXTURE_2D_ARRAY_SNORM:
            return "2DArray_snorm_float4_";
        case HLSL_TEXTURE_3D_SNORM:
            return "3D_snorm_float4_";
        case HLSL_TEXTURE_2D_MS:
            return "2DMS";
        case HLSL_TEXTURE_2D_INT4:
            return "2D_int4_";
        case HLSL_TEXTURE_3D_INT4:
            return "3D_int4_";
        case HLSL_TEXTURE_2D_ARRAY_INT4:
            return "2DArray_int4_";
        case HLSL_TEXTURE_2D_MS_INT4:
            return "2DMS_int4_";
        case HLSL_TEXTURE_2D_UINT4:
            return "2D_uint4_";
        case HLSL_TEXTURE_3D_UINT4:
            return "3D_uint4_";
        case HLSL_TEXTURE_2D_ARRAY_UINT4:
            return "2DArray_uint4_";
        case HLSL_TEXTURE_2D_MS_UINT4:
            return "2DMS_uint4_";
        case HLSL_TEXTURE_2D_COMPARISON:
            return "2D_comparison";
        case HLSL_TEXTURE_CUBE_COMPARISON:
            return "Cube_comparison";
        case HLSL_TEXTURE_2D_ARRAY_COMPARISON:
            return "2DArray_comparison";
        default:
            UNREACHABLE();
    }

    return "<unknown texture type>";
}

const char *TextureGroupSuffix(const TBasicType type,
                               TLayoutImageInternalFormat imageInternalFormat)
{
    return TextureGroupSuffix(TextureGroup(type, imageInternalFormat));
}

const char *TextureTypeSuffix(const TBasicType type, TLayoutImageInternalFormat imageInternalFormat)
{
    switch (type)
    {
        case EbtISamplerCube:
            return "Cube_int4_";
        case EbtUSamplerCube:
            return "Cube_uint4_";
        case EbtSamplerExternalOES:
            return "_External";
        case EbtImageCube:
        {
            switch (imageInternalFormat)
            {
                case EiifRGBA32F:
                case EiifRGBA16F:
                case EiifR32F:
                    return "Cube_float4_";
                case EiifRGBA8:
                    return "Cube_unorm_float4_";
                case EiifRGBA8_SNORM:
                    return "Cube_snorm_float4_";
                default:
                    UNREACHABLE();
            }
#if !UNREACHABLE_IS_NORETURN
            break;
#endif
        }
        case EbtIImageCube:
        {
            switch (imageInternalFormat)
            {
                case EiifRGBA32I:
                case EiifRGBA16I:
                case EiifRGBA8I:
                case EiifR32I:
                    return "Cube_int4_";
                default:
                    UNREACHABLE();
            }
#if !UNREACHABLE_IS_NORETURN
            break;
#endif
        }
        case EbtUImageCube:
        {
            switch (imageInternalFormat)
            {
                case EiifRGBA32UI:
                case EiifRGBA16UI:
                case EiifRGBA8UI:
                case EiifR32UI:
                    return "Cube_uint4_";
                default:
                    UNREACHABLE();
            }
#if !UNREACHABLE_IS_NORETURN
            break;
#endif
        }
        default:
            // All other types are identified by their group suffix
            return TextureGroupSuffix(type, imageInternalFormat);
    }
#if !UNREACHABLE_IS_NORETURN
    UNREACHABLE();
    return "_TTS_invalid_";
#endif
}

HLSLRWTextureGroup RWTextureGroup(const TBasicType type,
                                  TLayoutImageInternalFormat imageInternalFormat)

{
    switch (type)
    {
        case EbtImage2D:
        {
            switch (imageInternalFormat)
            {
                case EiifRGBA32F:
                case EiifRGBA16F:
                case EiifR32F:
                    return HLSL_RWTEXTURE_2D_FLOAT4;
                case EiifRGBA8:
                    return HLSL_RWTEXTURE_2D_UNORM;
                case EiifRGBA8_SNORM:
                    return HLSL_RWTEXTURE_2D_SNORM;
                default:
                    UNREACHABLE();
            }
#if !UNREACHABLE_IS_NORETURN
            break;
#endif
        }
        case EbtIImage2D:
        {
            switch (imageInternalFormat)
            {
                case EiifRGBA32I:
                case EiifRGBA16I:
                case EiifRGBA8I:
                case EiifR32I:
                    return HLSL_RWTEXTURE_2D_INT4;
                default:
                    UNREACHABLE();
            }
#if !UNREACHABLE_IS_NORETURN
            break;
#endif
        }
        case EbtUImage2D:
        {
            switch (imageInternalFormat)
            {

                case EiifRGBA32UI:
                case EiifRGBA16UI:
                case EiifRGBA8UI:
                case EiifR32UI:
                    return HLSL_RWTEXTURE_2D_UINT4;
                default:
                    UNREACHABLE();
            }
#if !UNREACHABLE_IS_NORETURN
            break;
#endif
        }
        case EbtImage3D:
        {
            switch (imageInternalFormat)
            {
                case EiifRGBA32F:
                case EiifRGBA16F:
                case EiifR32F:
                    return HLSL_RWTEXTURE_3D_FLOAT4;
                case EiifRGBA8:
                    return HLSL_RWTEXTURE_3D_UNORM;
                case EiifRGBA8_SNORM:
                    return HLSL_RWTEXTURE_3D_SNORM;
                default:
                    UNREACHABLE();
            }
#if !UNREACHABLE_IS_NORETURN
            break;
#endif
        }
        case EbtIImage3D:
        {
            switch (imageInternalFormat)
            {
                case EiifRGBA32I:
                case EiifRGBA16I:
                case EiifRGBA8I:
                case EiifR32I:
                    return HLSL_RWTEXTURE_3D_INT4;
                default:
                    UNREACHABLE();
            }
#if !UNREACHABLE_IS_NORETURN
            break;
#endif
        }
        case EbtUImage3D:
        {
            switch (imageInternalFormat)
            {
                case EiifRGBA32UI:
                case EiifRGBA16UI:
                case EiifRGBA8UI:
                case EiifR32UI:
                    return HLSL_RWTEXTURE_3D_UINT4;
                default:
                    UNREACHABLE();
            }
#if !UNREACHABLE_IS_NORETURN
            break;
#endif
        }
        case EbtImage2DArray:
        case EbtImageCube:
        {
            switch (imageInternalFormat)
            {
                case EiifRGBA32F:
                case EiifRGBA16F:
                case EiifR32F:
                    return HLSL_RWTEXTURE_2D_ARRAY_FLOAT4;
                case EiifRGBA8:
                    return HLSL_RWTEXTURE_2D_ARRAY_UNORN;
                case EiifRGBA8_SNORM:
                    return HLSL_RWTEXTURE_2D_ARRAY_SNORM;
                default:
                    UNREACHABLE();
            }
#if !UNREACHABLE_IS_NORETURN
            break;
#endif
        }
        case EbtIImage2DArray:
        case EbtIImageCube:
        {
            switch (imageInternalFormat)
            {
                case EiifRGBA32I:
                case EiifRGBA16I:
                case EiifRGBA8I:
                case EiifR32I:
                    return HLSL_RWTEXTURE_2D_ARRAY_INT4;
                default:
                    UNREACHABLE();
            }
#if !UNREACHABLE_IS_NORETURN
            break;
#endif
        }
        case EbtUImage2DArray:
        case EbtUImageCube:
        {
            switch (imageInternalFormat)
            {
                case EiifRGBA32UI:
                case EiifRGBA16UI:
                case EiifRGBA8UI:
                case EiifR32UI:
                    return HLSL_RWTEXTURE_2D_ARRAY_UINT4;
                default:
                    UNREACHABLE();
            }
#if !UNREACHABLE_IS_NORETURN
            break;
#endif
        }
        default:
            UNREACHABLE();
    }
    return HLSL_RWTEXTURE_UNKNOWN;
}

const char *RWTextureString(const HLSLRWTextureGroup RWTextureGroup)
{
    switch (RWTextureGroup)
    {
        case HLSL_RWTEXTURE_2D_FLOAT4:
            return "RWTexture2D<float4>";
        case HLSL_RWTEXTURE_2D_ARRAY_FLOAT4:
            return "RWTexture2DArray<float4>";
        case HLSL_RWTEXTURE_3D_FLOAT4:
            return "RWTexture3D<float4>";
        case HLSL_RWTEXTURE_2D_UNORM:
            return "RWTexture2D<unorm float4>";
        case HLSL_RWTEXTURE_2D_ARRAY_UNORN:
            return "RWTexture2DArray<unorm float4>";
        case HLSL_RWTEXTURE_3D_UNORM:
            return "RWTexture3D<unorm float4>";
        case HLSL_RWTEXTURE_2D_SNORM:
            return "RWTexture2D<snorm float4>";
        case HLSL_RWTEXTURE_2D_ARRAY_SNORM:
            return "RWTexture2DArray<snorm float4>";
        case HLSL_RWTEXTURE_3D_SNORM:
            return "RWTexture3D<snorm float4>";
        case HLSL_RWTEXTURE_2D_UINT4:
            return "RWTexture2D<uint4>";
        case HLSL_RWTEXTURE_2D_ARRAY_UINT4:
            return "RWTexture2DArray<uint4>";
        case HLSL_RWTEXTURE_3D_UINT4:
            return "RWTexture3D<uint4>";
        case HLSL_RWTEXTURE_2D_INT4:
            return "RWTexture2D<int4>";
        case HLSL_RWTEXTURE_2D_ARRAY_INT4:
            return "RWTexture2DArray<int4>";
        case HLSL_RWTEXTURE_3D_INT4:
            return "RWTexture3D<int4>";
        default:
            UNREACHABLE();
    }

    return "<unknown read and write texture type>";
}

const char *RWTextureString(const TBasicType type, TLayoutImageInternalFormat imageInternalFormat)
{
    return RWTextureString(RWTextureGroup(type, imageInternalFormat));
}

const char *RWTextureGroupSuffix(const HLSLRWTextureGroup type)
{
    switch (type)
    {
        case HLSL_RWTEXTURE_2D_FLOAT4:
            return "RW2D_float4_";
        case HLSL_RWTEXTURE_2D_ARRAY_FLOAT4:
            return "RW2DArray_float4_";
        case HLSL_RWTEXTURE_3D_FLOAT4:
            return "RW3D_float4_";
        case HLSL_RWTEXTURE_2D_UNORM:
            return "RW2D_unorm_float4_";
        case HLSL_RWTEXTURE_2D_ARRAY_UNORN:
            return "RW2DArray_unorm_float4_";
        case HLSL_RWTEXTURE_3D_UNORM:
            return "RW3D_unorm_float4_";
        case HLSL_RWTEXTURE_2D_SNORM:
            return "RW2D_snorm_float4_";
        case HLSL_RWTEXTURE_2D_ARRAY_SNORM:
            return "RW2DArray_snorm_float4_";
        case HLSL_RWTEXTURE_3D_SNORM:
            return "RW3D_snorm_float4_";
        case HLSL_RWTEXTURE_2D_UINT4:
            return "RW2D_uint4_";
        case HLSL_RWTEXTURE_2D_ARRAY_UINT4:
            return "RW2DArray_uint4_";
        case HLSL_RWTEXTURE_3D_UINT4:
            return "RW3D_uint4_";
        case HLSL_RWTEXTURE_2D_INT4:
            return "RW2D_int4_";
        case HLSL_RWTEXTURE_2D_ARRAY_INT4:
            return "RW2DArray_int4_";
        case HLSL_RWTEXTURE_3D_INT4:
            return "RW3D_int4_";
        default:
            UNREACHABLE();
    }

    return "<unknown read and write resource>";
}

const char *RWTextureGroupSuffix(const TBasicType type,
                                 TLayoutImageInternalFormat imageInternalFormat)
{
    return RWTextureGroupSuffix(RWTextureGroup(type, imageInternalFormat));
}

const char *RWTextureTypeSuffix(const TBasicType type,
                                TLayoutImageInternalFormat imageInternalFormat)
{
    switch (type)
    {
        case EbtImageCube:
        {
            switch (imageInternalFormat)
            {
                case EiifRGBA32F:
                case EiifRGBA16F:
                case EiifR32F:
                    return "RWCube_float4_";
                case EiifRGBA8:
                    return "RWCube_unorm_float4_";
                case EiifRGBA8_SNORM:
                    return "RWCube_unorm_float4_";
                default:
                    UNREACHABLE();
            }
#if !UNREACHABLE_IS_NORETURN
            break;
#endif
        }
        case EbtIImageCube:
        {
            switch (imageInternalFormat)
            {
                case EiifRGBA32I:
                case EiifRGBA16I:
                case EiifRGBA8I:
                case EiifR32I:
                    return "RWCube_int4_";
                default:
                    UNREACHABLE();
            }
#if !UNREACHABLE_IS_NORETURN
            break;
#endif
        }
        case EbtUImageCube:
        {
            switch (imageInternalFormat)
            {
                case EiifRGBA32UI:
                case EiifRGBA16UI:
                case EiifRGBA8UI:
                case EiifR32UI:
                    return "RWCube_uint4_";
                default:
                    UNREACHABLE();
            }
#if !UNREACHABLE_IS_NORETURN
            break;
#endif
        }
        default:
            // All other types are identified by their group suffix
            return TextureGroupSuffix(type, imageInternalFormat);
    }
#if !UNREACHABLE_IS_NORETURN
    UNREACHABLE();
    return "_RWTS_invalid_";
#endif
}

ImmutableString DecorateField(const ImmutableString &string, const TStructure &structure)
{
    if (structure.symbolType() != SymbolType::BuiltIn)
    {
        return Decorate(string);
    }

    return string;
}

ImmutableString Decorate(const ImmutableString &string)
{
    if (!string.beginsWith("gl_"))
    {
        ImmutableStringBuilder decorated(1u + string.length());
        decorated << "_" << string;
        return decorated;
    }

    return string;
}

ImmutableString DecorateVariableIfNeeded(const TVariable &variable)
{
    if (variable.symbolType() == SymbolType::AngleInternal ||
        variable.symbolType() == SymbolType::Empty)
    {
        // Besides handling internal variables, we generate names for nameless parameters here.
        const ImmutableString &name = variable.name();
        // The name should not have a prefix reserved for user-defined variables or functions.
        ASSERT(!name.beginsWith("f_"));
        ASSERT(!name.beginsWith("_"));
        return name;
    }
    else
    {
        return Decorate(variable.name());
    }
}

ImmutableString DecorateFunctionIfNeeded(const TFunction *func)
{
    if (func->symbolType() == SymbolType::AngleInternal)
    {
        // The name should not have a prefix reserved for user-defined variables or functions.
        ASSERT(!func->name().beginsWith("f_"));
        ASSERT(!func->name().beginsWith("_"));
        return func->name();
    }
    ASSERT(!func->name().beginsWith("gl_"));
    // Add an additional f prefix to functions so that they're always disambiguated from variables.
    // This is necessary in the corner case where a variable declaration hides a function that it
    // uses in its initializer.
    ImmutableStringBuilder decorated(2u + func->name().length());
    decorated << "f_" << func->name();
    return decorated;
}

ImmutableString TypeString(const TType &type)
{
    const TStructure *structure = type.getStruct();
    if (structure)
    {
        if (structure->symbolType() != SymbolType::Empty)
        {
            return StructNameString(*structure);
        }
        // Nameless structure, define in place
        TString namelessDef = StructureHLSL::defineNameless(*structure);
        return ImmutableString(AllocatePoolCharArray(namelessDef.c_str(), namelessDef.length()),
                               namelessDef.length());
    }
    if (type.isMatrix())
    {
        switch (type.getCols())
        {
            case 2:
                switch (type.getRows())
                {
                    case 2:
                        return ImmutableString("float2x2");
                    case 3:
                        return ImmutableString("float2x3");
                    case 4:
                        return ImmutableString("float2x4");
                    default:
                        break;
                }
                break;
            case 3:
                switch (type.getRows())
                {
                    case 2:
                        return ImmutableString("float3x2");
                    case 3:
                        return ImmutableString("float3x3");
                    case 4:
                        return ImmutableString("float3x4");
                    default:
                        break;
                }
                break;
            case 4:
                switch (type.getRows())
                {
                    case 2:
                        return ImmutableString("float4x2");
                    case 3:
                        return ImmutableString("float4x3");
                    case 4:
                        return ImmutableString("float4x4");
                    default:
                        break;
                }
                break;
        }
        UNREACHABLE();
        return ImmutableString("float4x4");
    }
    else
    {
        switch (type.getBasicType())
        {
            case EbtFloat:
                return kFloatTypeNames[type.getNominalSize()];
            case EbtInt:
                return kIntTypeNames[type.getNominalSize()];
            case EbtUInt:
                return kUIntTypeNames[type.getNominalSize()];
            case EbtBool:
                return kBoolTypeNames[type.getNominalSize()];
            case EbtVoid:
                return ImmutableString("void");
            case EbtSampler2D:
            case EbtISampler2D:
            case EbtUSampler2D:
            case EbtSampler2DArray:
            case EbtISampler2DArray:
            case EbtUSampler2DArray:
                return ImmutableString("sampler2D");
            case EbtSamplerCube:
            case EbtISamplerCube:
            case EbtUSamplerCube:
                return ImmutableString("samplerCUBE");
            case EbtSamplerExternalOES:
                return ImmutableString("sampler2D");
            case EbtAtomicCounter:
                return ImmutableString("atomic_uint");
            default:
                break;
        }
    }

    UNREACHABLE();
    return ImmutableString("<unknown type>");
}

ImmutableString StructNameString(const TStructure &structure)
{
    if (structure.symbolType() == SymbolType::Empty)
    {
        return ImmutableString("");
    }

    // For structures at global scope we use a consistent
    // translation so that we can link between shader stages.
    if (structure.atGlobalScope())
    {
        return Decorate(structure.name());
    }

    const ImmutableString &originalName = structure.name();

    ImmutableStringBuilder structName(11u + originalName.length());
    structName << "ss";
    structName.appendHex(structure.uniqueId().get());
    structName << "_" << originalName;

    return structName;
}

ImmutableString QualifiedStructNameString(const TStructure &structure,
                                          bool useHLSLRowMajorPacking,
                                          bool useStd140Packing)
{
    if (structure.symbolType() == SymbolType::Empty)
    {
        return ImmutableString("");
    }

    const ImmutableString &structName = StructNameString(structure);

    ImmutableStringBuilder qualifiedStructName(structName.length() + 7u);

    // Structs packed with row-major matrices in HLSL are prefixed with "rm"
    // GLSL column-major maps to HLSL row-major, and the converse is true

    if (useStd140Packing)
    {
        qualifiedStructName << "std_";
    }
    if (useHLSLRowMajorPacking)
    {
        qualifiedStructName << "rm_";
    }

    qualifiedStructName << structName;

    return qualifiedStructName;
}

const char *InterpolationString(TQualifier qualifier)
{
    switch (qualifier)
    {
        case EvqVaryingIn:
            return "";
        case EvqFragmentIn:
            return "";
        case EvqSmoothIn:
            return "linear";
        case EvqFlatIn:
            return "nointerpolation";
        case EvqCentroidIn:
            return "centroid";
        case EvqVaryingOut:
            return "";
        case EvqVertexOut:
            return "";
        case EvqSmoothOut:
            return "linear";
        case EvqFlatOut:
            return "nointerpolation";
        case EvqCentroidOut:
            return "centroid";
        default:
            UNREACHABLE();
    }

    return "";
}

const char *QualifierString(TQualifier qualifier)
{
    switch (qualifier)
    {
        case EvqIn:
            return "in";
        case EvqOut:
            return "inout";  // 'out' results in an HLSL error if not all fields are written, for
                             // GLSL it's undefined
        case EvqInOut:
            return "inout";
        case EvqConstReadOnly:
            return "const";
        default:
            UNREACHABLE();
    }

    return "";
}

TString DisambiguateFunctionName(const TFunction *func)
{
    TStringStream disambiguatingString;
    size_t paramCount = func->getParamCount();
    for (size_t i = 0; i < paramCount; ++i)
    {
        DisambiguateFunctionNameForParameterType(func->getParam(i)->getType(),
                                                 &disambiguatingString);
    }
    return disambiguatingString.str();
}

TString DisambiguateFunctionName(const TIntermSequence *args)
{
    TStringStream disambiguatingString;
    for (TIntermNode *arg : *args)
    {
        ASSERT(arg->getAsTyped());
        DisambiguateFunctionNameForParameterType(arg->getAsTyped()->getType(),
                                                 &disambiguatingString);
    }
    return disambiguatingString.str();
}

}  // namespace sh
