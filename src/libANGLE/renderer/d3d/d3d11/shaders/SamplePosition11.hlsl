//--------------------------------------------------------------------------------------
// File: Tutorial07.fx
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------
Texture2DMS<float4> txDiffuse : register(t0);
SamplerState samLinear : register(s0);
//RWBuffer<float> BufferOut : register(u1);
struct BufType
{
	float x, y;
};

RWStructuredBuffer<BufType> BufferOut: register(u1);

cbuffer cbNeverChanges : register(b0)
{
	matrix View;
};

cbuffer cbChangeOnResize : register(b1)
{
	matrix Projection;
};

cbuffer cbChangesEveryFrame : register(b2)
{
	matrix World;
	float4 vMeshColor;
};


//--------------------------------------------------------------------------------------
struct VS_INPUT
{
	float2 Pos : POSITION;
	float2 Tex : TEXCOORD0;
};

struct PS_INPUT
{
	float4 Pos : SV_POSITION;
	float2 Tex : TEXCOORD0;
};


//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
PS_INPUT VS_SamplePosition(VS_INPUT input)
{
	PS_INPUT output = (PS_INPUT)0;
	output.Pos = float4(input.Pos, 0.0f, 1.0f);
	output.Tex = input.Tex;

	return output;
}


//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS_SamplePosition_2F(PS_INPUT input, uint sampleIndex: SV_SampleIndex) : SV_Target
{
	//return txDiffuse.Load(input.Tex, sampleIndex) * vMeshColor;
	float2 location = txDiffuse.GetSamplePosition(1);
	BufferOut[0].x = 1.0;
	BufferOut[0].y = 2.0;
	return float4(location.x, location.y, 0, 1);
}
