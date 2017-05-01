Texture2D<float4> TextureF  : register(t0);
Texture2D<uint4>  TextureUI : register(t0);
Texture2D<int4>   TextureI  : register(t0);

SamplerState Sampler        : register(s0);

// Cover pre-multiply and un-multiply alpha of the following permutations of types:
// Input and output types: float, uint, int
// Permutations (for each type x): a) float -> x
//                                 b) x -> x
// Special cases for writing to some LUMA and RGB formats because they are emulated

// Notation:
// PM: premultiply, UM: unmulitply, PT: passthrough
// F: float, U: uint, I: int

// Float to float RGBA
float4 PS_FtoF_PM_RGBA(in float4 inPosition : SV_POSITION, in float2 inTexCoord : TEXCOORD0) : SV_TARGET0
{
    float4 color = TextureF.Sample(Sampler, inTexCoord).rgba;
    color.rgb *= color.a;
    return color;
}

float4 PS_FtoF_UM_RGBA(in float4 inPosition : SV_POSITION, in float2 inTexCoord : TEXCOORD0) : SV_TARGET0
{
    float4 color = TextureF.Sample(Sampler, inTexCoord).rgba;
    if (color.a > 0.0f)
    {
        color.rgb /= color.a;
    }
    return color;
}

// Float to uint RGBA
uint4 PS_FtoU_PT_RGBA(in float4 inPosition : SV_POSITION, in float2 inTexCoord : TEXCOORD0) : SV_TARGET0
{
    float4 color = TextureF.Sample(Sampler, inTexCoord).rgba;
    return uint4(color * 255);
}

uint4 PS_FtoU_PM_RGBA(in float4 inPosition : SV_POSITION, in float2 inTexCoord : TEXCOORD0) : SV_TARGET0
{
    float4 color = TextureF.Sample(Sampler, inTexCoord).rgba;
    color.rgb *= color.a;
    return uint4(color * 255);
}

uint4 PS_FtoU_UM_RGBA(in float4 inPosition : SV_POSITION, in float2 inTexCoord : TEXCOORD0) : SV_TARGET0
{
    float4 color = TextureF.Sample(Sampler, inTexCoord).rgba;
    if (color.a > 0.0f)
    {
        color.rgb /= color.a;
    }
    return uint4(color * 255);
}

// Float to int RGBA
int4 PS_FtoI_PT_RGBA(in float4 inPosition : SV_POSITION, in float2 inTexCoord : TEXCOORD0) : SV_TARGET0
{
    float4 color = TextureF.Sample(Sampler, inTexCoord).rgba;
    return int4(color * 255);
}

int4 PS_FtoI_PM_RGBA(in float4 inPosition : SV_POSITION, in float2 inTexCoord : TEXCOORD0) : SV_TARGET0
{
    float4 color = TextureF.Sample(Sampler, inTexCoord).rgba;
    color.rgb *= color.a;
    return int4(color * 255);
}

int4 PS_FtoI_UM_RGBA(in float4 inPosition : SV_POSITION, in float2 inTexCoord : TEXCOORD0) : SV_TARGET0
{
    float4 color = TextureF.Sample(Sampler, inTexCoord).rgba;
    if (color.a > 0.0f)
    {
        color.rgb /= color.a;
    }
    return int4(color * 255);
}

// uint to uint RGBA
uint4 PS_UtoU_PM_RGBA(in float4 inPosition : SV_POSITION, in float2 inTexCoord : TEXCOORD0) : SV_TARGET0
{
    uint2 size;
    TextureUI.GetDimensions(size.x, size.y);
    uint4 color = TextureUI.Load(int3(size * inTexCoord, 0)).rgba;
    color.rgb *= color.a;
    color.rgb /= 255;
    return color;
}

uint4 PS_UtoU_UM_RGBA(in float4 inPosition : SV_POSITION, in float2 inTexCoord : TEXCOORD0) : SV_TARGET0
{
    uint2 size;
    TextureUI.GetDimensions(size.x, size.y);
    uint4 color = TextureUI.Load(int3(size * inTexCoord, 0)).rgba;
    if (color.a > 0)
    {
        color.rgb /= color.a;
        color.rgb *= 255;
    }
    return color;
}

// int to int rgba
int4 PS_ItoI_PM_RGBA(in float4 inPosition : SV_POSITION, in float2 inTexCoord : TEXCOORD0) : SV_TARGET0
{
    uint2 size;
    TextureI.GetDimensions(size.x, size.y);
    int4 color = TextureI.Load(int3(size * inTexCoord, 0)).rgba;
    color.rgb *= color.a;
    color.rgb /= 255;
    return color;
}

int4 PS_ItoI_UM_RGBA(in float4 inPosition : SV_POSITION, in float2 inTexCoord : TEXCOORD0) : SV_TARGET0
{
    uint2 size;
    TextureI.GetDimensions(size.x, size.y);
    int4 color = TextureI.Load(int3(size * inTexCoord, 0)).rgba;
    if (color.a > 0)
    {
        color.rgb /= color.a;
        color.rgb *= 255;
    }
    return color;
}