Texture2DMS<float4> TextureMSF  : register(t0);
Texture2DMS<uint4>  TextureMSUI : register(t0);
Texture2DMS<int4>   TextureMSI  : register(t0);

/*cbuffer SamplePositionIndex     : register(b0)
{
    int4 index;
};*/

struct BufType
{
	float x, y;
};
RWStructuredBuffer<BufType> BufferOut : register(u1);

void VS_SamplePosition(in float2  inPosition :    POSITION, in float2  inTexCoord : TEXCOORD0,
	out float4 outPosition : SV_POSITION, out float2 outTexCoord : TEXCOORD0)
{
	outPosition = float4(inPosition, 0.0f, 1.0f);
	outTexCoord = inTexCoord;
}

float4 PS_SamplePosition_2F(in float4 inPosition : SV_POSITION, in float2 inTexCoord : TEXCOORD0) : SV_TARGET0
{
	uint width, height, sampleCount;
	TextureMSF.GetDimensions(width, height, sampleCount);
	for (uint index = 0; index < sampleCount; index++)
	{
		float2 location = TextureMSF.GetSamplePosition(index);
		BufferOut[index].x = location.x;
		BufferOut[index].y = location.y;
	}
    return TextureMSF.Load(inTexCoord, 0, 0);
}

float4 PS_SamplePosition_2UI(in float4 inPosition : SV_POSITION, in float2 inTexCoord : TEXCOORD0) : SV_TARGET0
{
	uint width, height, sampleCount;
    TextureMSUI.GetDimensions(width, height, sampleCount);
	for (uint index = 0; index < sampleCount; index++)
	{
		float2 location = TextureMSUI.GetSamplePosition(index);
		BufferOut[index].x = location.x;
		BufferOut[index].y = location.y;
	}
    return TextureMSUI.Load(inTexCoord, 0, 0);
}

float4 PS_SamplePosition_2I(in float4 inPosition : SV_POSITION, in float2 inTexCoord : TEXCOORD0) : SV_TARGET0
{
	uint width, height, sampleCount;
    TextureMSUI.GetDimensions(width, height, sampleCount);
	for (uint index = 0; index < sampleCount; index++)
	{
		float2 location = TextureMSUI.GetSamplePosition(index);
		BufferOut[index].x = location.x;
		BufferOut[index].y = location.y;
	}
	return TextureMSUI.Load(inTexCoord, 0, 0);
}
