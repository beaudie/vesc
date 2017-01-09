Texture2DMS<float4> TextureMSF  : register(t0);
Texture2DMS<uint4>  TextureMSUI : register(t0);
Texture2DMS<int4>   TextureMSI  : register(t0);

struct SamplePosition
{
    float x, y;
};
RWStructuredBuffer<SamplePosition>  SamplePositionsBuffer : register(u1);

void VS_SamplePosition(in float2  inPosition :    POSITION, in float2  inTexCoord : TEXCOORD0,
    out float4 outPosition : SV_POSITION, out float2 outTexCoord : TEXCOORD0)
{
    outPosition = float4(inPosition, 0.0f, 1.0f);
    outTexCoord = inTexCoord;
}

void PS_SamplePosition_2F(in float4 inPosition : SV_POSITION, in float2 inTexCoord : TEXCOORD0)
{
    uint width, height, sampleCount;
    TextureMSF.GetDimensions(width, height, sampleCount);
    for (uint index = 0; index < sampleCount; index++)
    {
        float2 location = TextureMSF.GetSamplePosition(index);
        SamplePositionsBuffer[index].x = location.x;
        SamplePositionsBuffer[index].y = location.y;
    }
}

void PS_SamplePosition_2UI(in float4 inPosition : SV_POSITION, in float2 inTexCoord : TEXCOORD0)
{
    uint width, height, sampleCount;
    TextureMSUI.GetDimensions(width, height, sampleCount);
    for (uint index = 0; index < sampleCount; index++)
    {
        float2 location = TextureMSUI.GetSamplePosition(index);
        SamplePositionsBuffer[index].x = location.x;
        SamplePositionsBuffer[index].y = location.y;
    }
}

void PS_SamplePosition_2I(in float4 inPosition : SV_POSITION, in float2 inTexCoord : TEXCOORD0)
{
    uint width, height, sampleCount;
    TextureMSUI.GetDimensions(width, height, sampleCount);
    for (uint index = 0; index < sampleCount; index++)
    {
        float2 location = TextureMSUI.GetSamplePosition(index);
        SamplePositionsBuffer[index].x = location.x;
        SamplePositionsBuffer[index].y = location.y;
    }
}
