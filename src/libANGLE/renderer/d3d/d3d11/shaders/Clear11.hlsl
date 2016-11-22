//
// Copyright (c) 2017 The ANGLE Project. All rights reserved.
// Copyright (c) 2017 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// Clear11.hlsl: Vertex and Pixel shaders for clearing RTVs and DSVs using
// draw calls and specifying float depth values and clear colors as either
// float, uint or sint. Notes:
//  - UINT & SINT clears can only be compiled with FL10+.
//  - VS_ClearAnyType_FL9 requires a VB to be bound with vertices
//    defining a quad covering entire surface (in clip co-ordinates)
//  - VS_ClearAnyType_FL9 used for all pixel shaders defined here


/// Vertex Shader
/// TODO: Use SV_VertexID to generate quad (for FL10+) to 
/// avoid having to rely on a VB to be generated/bound
void VS_ClearAnyType_FL9( in float4 inPosition : POSITION,
                          out float4 outPosition : SV_POSITION)
{
    outPosition = inPosition;
}

/// Pixel Shader Constant Buffers
cbuffer ColorAndDepthDataFloat : register(b0)
{
    float4 color_FloatCb   : packoffset(c0);
    float  zValueF_FloatCb : packoffset(c1.x);
    float  a1_FloatCb      : packoffset(c1.y);
    float  a2_FloatCb      : packoffset(c1.z);
    float  a3_FloatCb      : packoffset(c1.w);
    float  a4_FloatCb      : packoffset(c2.x);
    float  a5_FloatCb      : packoffset(c2.y);
    float  a6_FloatCb      : packoffset(c2.z);
    float  a7_FloatCb      : packoffset(c2.w);
}

cbuffer ColorAndDepthDataSint : register(b0)
{
    int4 color_SintCb    : packoffset(c0);
    float zValueF_SintCb : packoffset(c1.x);
}

cbuffer ColorAndDepthDataUint : register(b0)
{
    uint4 color_UintCb   : packoffset(c0);
    float zValueF_UintCb : packoffset(c1.x);
}

/// Pixel Shader Output Structs
struct PS_OutputFloat_FL9
{
    float4 color0 : SV_TARGET0;
    float4 color1 : SV_TARGET1;
    float4 color2 : SV_TARGET2;
    float4 color3 : SV_TARGET3;
    float  depth  : SV_DEPTH;
};

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

struct PS_OutputUint
{
    uint4 color0 : SV_TARGET0;
    uint4 color1 : SV_TARGET1;
    uint4 color2 : SV_TARGET2;
    uint4 color3 : SV_TARGET3;
    uint4 color4 : SV_TARGET4;
    uint4 color5 : SV_TARGET5;
    uint4 color6 : SV_TARGET6;
    uint4 color7 : SV_TARGET7;
    float depth  : SV_DEPTH;
};

struct PS_OutputSint
{
    int4 color0 : SV_TARGET0;
    int4 color1 : SV_TARGET1;
    int4 color2 : SV_TARGET2;
    int4 color3 : SV_TARGET3;
    int4 color4 : SV_TARGET4;
    int4 color5 : SV_TARGET5;
    int4 color6 : SV_TARGET6;
    int4 color7 : SV_TARGET7;
    float depth : SV_DEPTH;
};

/// Pixel Shaders (FL_9)
PS_OutputFloat_FL9 PS_ClearFloat_FL9(in float4 inPosition : SV_POSITION)
{
    PS_OutputFloat_FL9 outData;
    outData.color0 = color_FloatCb;
    outData.color1 = color_FloatCb;
    outData.color2 = color_FloatCb;
    outData.color3 = color_FloatCb;
    outData.depth  = zValueF_FloatCb;
    return outData;
}

PS_OutputFloat_FL9 PS_MultiColorClearFloat_FL9(in float4 inPosition : SV_POSITION)
{
    PS_OutputFloat_FL9 outColorsAndDepth;
    outColorsAndDepth.color0 = color_FloatCb;
    outColorsAndDepth.color1 = float4(color_FloatCb.xyz, a1_FloatCb);
    outColorsAndDepth.color2 = float4(color_FloatCb.xyz, a2_FloatCb);
    outColorsAndDepth.color3 = float4(color_FloatCb.xyz, a3_FloatCb);
    outColorsAndDepth.depth  = zValueF_FloatCb;
    return outColorsAndDepth;
}

/// Pixel Shaders (FL_10+)
PS_OutputFloat PS_ClearFloat(in float4 inPosition : SV_POSITION)
{
    PS_OutputFloat outData;
    outData.color0 = color_FloatCb;
    outData.color1 = color_FloatCb;
    outData.color2 = color_FloatCb;
    outData.color3 = color_FloatCb;
    outData.color4 = color_FloatCb;
    outData.color5 = color_FloatCb;
    outData.color6 = color_FloatCb;
    outData.color7 = color_FloatCb;
    outData.depth  = zValueF_FloatCb;
    return outData;
}

PS_OutputFloat PS_MultiColorClearFloat(in float4 inPosition : SV_POSITION)
{
    PS_OutputFloat outColorsAndDepth;
    outColorsAndDepth.color0 = color_FloatCb;
    outColorsAndDepth.color1 = float4(color_FloatCb.xyz, a1_FloatCb);
    outColorsAndDepth.color2 = float4(color_FloatCb.xyz, a2_FloatCb);
    outColorsAndDepth.color3 = float4(color_FloatCb.xyz, a3_FloatCb);
    outColorsAndDepth.color4 = float4(color_FloatCb.xyz, a4_FloatCb);
    outColorsAndDepth.color5 = float4(color_FloatCb.xyz, a5_FloatCb);
    outColorsAndDepth.color6 = float4(color_FloatCb.xyz, a6_FloatCb);
    outColorsAndDepth.color7 = float4(color_FloatCb.xyz, a7_FloatCb);
    outColorsAndDepth.depth  = zValueF_FloatCb;
    return outColorsAndDepth;
}

PS_OutputUint PS_ClearUint(in float4 inPosition : SV_POSITION)
{
    PS_OutputUint outData;
    outData.color0 = color_UintCb;
    outData.color1 = color_UintCb;
    outData.color2 = color_UintCb;
    outData.color3 = color_UintCb;
    outData.color4 = color_UintCb;
    outData.color5 = color_UintCb;
    outData.color6 = color_UintCb;
    outData.color7 = color_UintCb;
    outData.depth = zValueF_UintCb;
    return outData;
}

PS_OutputSint PS_ClearSint(in float4 inPosition : SV_POSITION)
{
    PS_OutputSint outData;
    outData.color0 = color_SintCb;
    outData.color1 = color_SintCb;
    outData.color2 = color_SintCb;
    outData.color3 = color_SintCb;
    outData.color4 = color_SintCb;
    outData.color5 = color_SintCb;
    outData.color6 = color_SintCb;
    outData.color7 = color_SintCb;
    outData.depth = zValueF_SintCb;
    return outData;
}
