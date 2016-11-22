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
//  - VS_ClearAnyType_FL9 used for all FL9 pixel shaders defined here
//  - VS_ClearAnyType used for all FL10+ pixel shaders defined here

/// Constants
static const float2 g_Corners[6] =
{
    float2(-1.0f,  1.0f),
    float2( 1.0f, -1.0f),
    float2(-1.0f, -1.0f),
    float2(-1.0f,  1.0f),
    float2( 1.0f,  1.0f),
    float2( 1.0f, -1.0f),
};

/// Vertex Shader (FL9 with VB input)
void VS_ClearAnyType_FL9( in float4 inPosition : POSITION,
                          out float4 outPosition : SV_POSITION)
{
    outPosition = inPosition;
}

/// Vertex Shader (FL10+ with vertex instancing and no VB input)
void VS_ClearAnyType( in uint id : SV_VertexID,
                      out float4 outPosition : SV_POSITION)
{
    float2 corner = g_Corners[id];
    outPosition = float4(corner.x, corner.y, 0.0f, 1.0f);
}

/// Pixel Shader Constant Buffers
cbuffer ColorAndDepthDataFloat : register(b0)
{
    float  zValueF_FloatCb     : packoffset(c0);
    float  Pad0F_FloatCb       : packoffset(c0.y);
    float  Pad1F_FloatCb       : packoffset(c0.z);
    float  Pad2F_FloatCb       : packoffset(c0.w);
    float4 clearColorF_FloatCb : packoffset(c1);
}

cbuffer ColorAndDepthDataSint : register(b0)
{
    float  zValueF_SintCb      : packoffset(c0);
    float  Pad0F_SintCb        : packoffset(c0.y);
    float  Pad1F_SintCb        : packoffset(c0.z);
    float  Pad2F_SintCb        : packoffset(c0.w);
    int4   clearColorSI_SintCb : packoffset(c1);
}

cbuffer ColorAndDepthDataUint : register(b0)
{
    float  zValueF_UintCb      : packoffset(c0);
    float  Pad0F_UintCb        : packoffset(c0.y);
    float  Pad1F_UintCb        : packoffset(c0.z);
    float  Pad2F_UintCb        : packoffset(c0.w);
    uint4  clearColorUI_UintCb : packoffset(c1);
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

/// Pixel Shader (FL_9)
PS_OutputFloat_FL9 PS_ClearFloat_FL9(in float4 inPosition : SV_POSITION)
{
    PS_OutputFloat_FL9 outData;
    outData.color0 = clearColorF_FloatCb;
    outData.color1 = clearColorF_FloatCb;
    outData.color2 = clearColorF_FloatCb;
    outData.color3 = clearColorF_FloatCb;
    outData.depth  = zValueF_FloatCb;
    return outData;
}

/// Pixel Shaders (FL_10+)
PS_OutputFloat PS_ClearFloat(in float4 inPosition : SV_POSITION)
{
    PS_OutputFloat outData;
    outData.color0 = clearColorF_FloatCb;
    outData.color1 = clearColorF_FloatCb;
    outData.color2 = clearColorF_FloatCb;
    outData.color3 = clearColorF_FloatCb;
    outData.color4 = clearColorF_FloatCb;
    outData.color5 = clearColorF_FloatCb;
    outData.color6 = clearColorF_FloatCb;
    outData.color7 = clearColorF_FloatCb;
    outData.depth  = zValueF_FloatCb;
    return outData;
}

PS_OutputUint PS_ClearUint(in float4 inPosition : SV_POSITION)
{
    PS_OutputUint outData;
    outData.color0 = clearColorUI_UintCb;
    outData.color1 = clearColorUI_UintCb;
    outData.color2 = clearColorUI_UintCb;
    outData.color3 = clearColorUI_UintCb;
    outData.color4 = clearColorUI_UintCb;
    outData.color5 = clearColorUI_UintCb;
    outData.color6 = clearColorUI_UintCb;
    outData.color7 = clearColorUI_UintCb;
    outData.depth = zValueF_UintCb;
    return outData;
}

PS_OutputSint PS_ClearSint(in float4 inPosition : SV_POSITION)
{
    PS_OutputSint outData;
    outData.color0 = clearColorSI_SintCb;
    outData.color1 = clearColorSI_SintCb;
    outData.color2 = clearColorSI_SintCb;
    outData.color3 = clearColorSI_SintCb;
    outData.color4 = clearColorSI_SintCb;
    outData.color5 = clearColorSI_SintCb;
    outData.color6 = clearColorSI_SintCb;
    outData.color7 = clearColorSI_SintCb;
    outData.depth = zValueF_SintCb;
    return outData;
}
