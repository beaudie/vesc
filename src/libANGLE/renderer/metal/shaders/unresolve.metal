//
// Copyright 2024 The ANGLE Project. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// unresolve.metal: Implements unresolve step by blitting texture content to current frame
// buffer.

#include "common.h"

using namespace rx::mtl_shader;

// Fragment's stencil output is only available since Metal 2.1
@@#if __METAL_VERSION__ >= 210

constant int kUnresolveFlags [[function_constant(3000)]];

constant bool kUnresolveColor0 = (kUnresolveFlags & 0x1) != 0;
constant bool kUnresolveColor1 = (kUnresolveFlags & 0x2) != 0;
constant bool kUnresolveColor2 = (kUnresolveFlags & 0x4) != 0;
constant bool kUnresolveColor3 = (kUnresolveFlags & 0x8) != 0;
constant bool kUnresolveColor4 = (kUnresolveFlags & 0x10) != 0;
constant bool kUnresolveColor5 = (kUnresolveFlags & 0x20) != 0;
constant bool kUnresolveColor6 = (kUnresolveFlags & 0x40) != 0;
constant bool kUnresolveColor7 = (kUnresolveFlags & 0x80) != 0;

constant bool kUnresolveDepth = (kUnresolveFlags & 0x100) != 0;
constant bool kUnresolveStencil = (kUnresolveFlags & 0x200) != 0;

struct UnresolveParams
{
    int srcColorLevels[8];

    int srcDepthLevel;
    
    int srcStencilLevel;
};

vertex float4 unresolveVS(unsigned int vid [[vertex_id]])
{
    float4 position;
    position.xy = select(float2(-1.0f), float2(1.0f), bool2(vid & uint2(2, 1)));
    position.zw = float2(0.0, 1.0);

    return position;
}

struct UnresolveFSOut
{
    float4 color0 [[color(0), function_constant(kUnresolveColor0)]];
    float4 color1 [[color(1), function_constant(kUnresolveColor1)]];
    float4 color2 [[color(2), function_constant(kUnresolveColor2)]];
    float4 color3 [[color(3), function_constant(kUnresolveColor3)]];
    float4 color4 [[color(4), function_constant(kUnresolveColor4)]];
    float4 color5 [[color(5), function_constant(kUnresolveColor5)]];
    float4 color6 [[color(6), function_constant(kUnresolveColor6)]];
    float4 color7 [[color(7), function_constant(kUnresolveColor7)]];
    float depth [[depth(any), function_constant(kUnresolveDepth)]];
    uint32_t stencil [[stencil, function_constant(kUnresolveStencil)]];
};

fragment UnresolveFSOut
unresolveFS(float4 fragPosition [[position]],
            texture2d<float> srcColor0
            [[texture(0), function_constant(kUnresolveColor0)]],
            texture2d<float> srcColor1
            [[texture(1), function_constant(kUnresolveColor1)]],
            texture2d<float> srcColor2
            [[texture(2), function_constant(kUnresolveColor2)]],
            texture2d<float> srcColor3
            [[texture(3), function_constant(kUnresolveColor3)]],
            texture2d<float> srcColor4
            [[texture(4), function_constant(kUnresolveColor4)]],
            texture2d<float> srcColor5
            [[texture(5), function_constant(kUnresolveColor5)]],
            texture2d<float> srcColor6
            [[texture(6), function_constant(kUnresolveColor6)]],
            texture2d<float> srcColor7
            [[texture(7), function_constant(kUnresolveColor7)]],
            texture2d<float> srcDepth
            [[texture(8), function_constant(kUnresolveDepth)]],
            texture2d<uint> srcStencil
            [[texture(9), function_constant(kUnresolveStencil)]],
            constant UnresolveParams &options [[buffer(0)]])
{
    UnresolveFSOut output;

    uint2 fragLocation = uint2(fragPosition.x, fragPosition.y);

    // Read color textures
#define UNRESOLVE_COLOR(i)                                                               \
    do                                                                                   \
    {                                                                                    \
        if (kUnresolveColor##i)                                                          \
        {                                                                                \
            output.color##i = srcColor##i.read(fragLocation, options.srcColorLevels[i]); \
        }                                                                                \
    } while (0)

    UNRESOLVE_COLOR(0);
    UNRESOLVE_COLOR(1);
    UNRESOLVE_COLOR(2);
    UNRESOLVE_COLOR(3);
    UNRESOLVE_COLOR(4);
    UNRESOLVE_COLOR(5);
    UNRESOLVE_COLOR(6);
    UNRESOLVE_COLOR(7);

#undef UNRESOLVE_COLOR

    // Read depth texture
    if (kUnresolveDepth)
    {
        output.depth = srcDepth.read(fragLocation, options.srcDepthLevel).r;
    }

    // Read stencil texture
    if (kUnresolveStencil)
    {
        output.stencil = srcStencil.read(fragLocation, options.srcStencilLevel).r;
    }

    return output;
}

@@#endif  // __METAL_VERSION__ >= 210
