//
// Copyright (c) 2017 The ANGLE Project. All rights reserved.
// Copyright (c) 2017 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// MultiColorClear11.hlsl: Pixel shader for clearing all bound RTVs and the
// DSV (if bound) with an explicit color is specified for each RTV.
// Dependencies: Feature level 10+

/// Pixel Shader Constant Buffers
cbuffer ColorAndDepthData : register(b0)
{
    float  zValue         : packoffset(c0);
    float  Pad0           : packoffset(c0.y);
    float  Pad1           : packoffset(c0.z);
    float  Pad2           : packoffset(c0.w);
    float4 clearColors[8] : packoffset(c1);
}

/// Pixel Shader Output Structs
struct PS_OutputFloat
{
    float4 color0 : SV_TARGET0;
    float4 color1 : SV_TARGET1;
    float4 color2 : SV_TARGET2;
    float4 color3 : SV_TARGET3;
    float4 color4 : SV_TARGET4;
    float4 color5 : SV_TARGET5;
    float4 color6 : SV_TARGET6;
    float4 color7 : SV_TARGET7;
    float  depth  : SV_DEPTH;
};

/// Pixel Shader
PS_OutputFloat PS_MultiColorClearFloat(in float4 inPosition : SV_POSITION)
{
    PS_OutputFloat outColorsAndDepth;
    outColorsAndDepth.color0 = clearColors[0];
    outColorsAndDepth.color1 = clearColors[1];
    outColorsAndDepth.color2 = clearColors[2];
    outColorsAndDepth.color3 = clearColors[3];
    outColorsAndDepth.color4 = clearColors[4];
    outColorsAndDepth.color5 = clearColors[5];
    outColorsAndDepth.color6 = clearColors[6];
    outColorsAndDepth.color7 = clearColors[7];
    outColorsAndDepth.depth  = zValue;
    return outColorsAndDepth;
}