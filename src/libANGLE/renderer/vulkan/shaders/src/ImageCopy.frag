//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ImageCopy.frag: Copy parts of an image to another.

#version 450 core

#extension GL_EXT_samplerless_texture_functions : require

#if SrcIsFloat
#define SRC_RESOURCE(type) type
#define SrcType vec4
#elif SrcIsInt
#define SRC_RESOURCE(type) i ## type
#define SrcType ivec4
#elif SrcIsUint
#define SRC_RESOURCE(type) u ## type
#define SrcType uvec4
#else
#error "Not all source formats are accounted for"
#endif

#if DestIsFloat
#define DestType vec4
#elif DestIsInt
#define DestType ivec4
#elif DestIsUint
#define DestType uvec4
#else
#error "Not all destinatoin formats are accounted for"
#endif

layout(set = 0, binding = 0) uniform SRC_RESOURCE(texture2D) src;
layout(location = 0) out DestType dest;

layout(push_constant) uniform PushConstants {
    // Whether y needs to be flipped
    bool flipY;
    // Whether source or destination is emulated luminance/alpha.
    bool srcIsLuma;
    bool srcIsAlpha;
    bool destIsLuma;
    bool destIsAlpha;
    // Whether each of these channels are present (or need to be set to default).  Note that if
    // either of source or destination is missing the channel, it will be set to default.  This
    // should prevent garbage from being read from the source's extra channels or written to dest's.
    // These extra channels are a result of format emulation.
    bool defaultRed;
    bool defaultGreen;
    bool defaultBlue;
    bool defaultAlpha;
    // Translation from source to destination coordinates.
    int srcMip;
    ivec2 srcOffset;
    ivec2 destOffset;
} params;

void main()
{
    ivec2 destSubImageCoords = ivec2(gl_FragCoord.xy) - params.destOffset;

    ivec2 srcSubImageCoords = destSubImageCoords;

    // If flipping Y, srcOffset would contain the opposite y coordinate, so we can
    // simply reverse the direction in which y grows.
    if (params.flipY)
        srcSubImageCoords.y = -srcSubImageCoords.y;

    SrcType srcValue = texelFetch(src, params.srcOffset + srcSubImageCoords, params.srcMip);

    SrcType srcDefault = SrcType(0, 0, 0, 1);
    DestType destDefault = DestType(0, 0, 0, 1);

    // If source is luminance/alpha, it's emulated with R or RG, so the appropriate swizzle needs to
    // happen.
    if (params.srcIsLuma)
    {
        srcValue.rgba = srcValue.rrrg;
    }
    else if (params.srcIsAlpha)
    {
        srcValue.a = srcValue.r;
        srcValue.rgb = srcDefault.rgb;
    }

    // Convert value to destination type.
    DestType destValue = DestType(srcValue);

    // Set missing channels to default, so that if they are emulated, they won't contain garbage.
    destValue.r = params.defaultRed ? destDefault.r : destValue.r;
    destValue.g = params.defaultGreen ? destDefault.g : destValue.g;
    destValue.b = params.defaultBlue ? destDefault.b : destValue.b;
    destValue.a = params.defaultAlpha ? destDefault.a : destValue.a;

    // If dest is luminance/alpha, do the appropriate swizzle.
    if (params.destIsLuma)
    {
        destValue.rg = destValue.ra;
        destValue.ba = destDefault.ba;
    }
    else if (params.destIsAlpha)
    {
        destValue.r = destValue.a;
        destValue.gba = destDefault.gba;
    }

    dest = destValue;
}
