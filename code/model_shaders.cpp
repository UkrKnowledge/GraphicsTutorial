
cbuffer RenderUniforms : register(b0)
{
    float4x4 WVPTransform;
};

Texture2D Texture : register(t0);
SamplerState BilinearSampler : register(s0);

struct vs_input
{
    float3 Position : POSITION;
    float2 Uv : TEXCOORD0;
};

struct ps_input
{
    float4 Position : SV_POSITION;
    float2 Uv : TEXCOORD0;
};

ps_input ModelVsMain(vs_input Input)
{
    ps_input Result;

    Result.Position = mul(WVPTransform, float4(Input.Position, 1.0f));
    Result.Uv = Input.Uv;

    return Result;
}

struct ps_output
{
    float4 Color : SV_TARGET0;
};

ps_output ModelPsMain(ps_input Input)
{
    ps_output Result;
    Result.Color = Texture.Sample(BilinearSampler, Input.Uv);
    if (Result.Color.a == 0.0f)
    {
        discard;
    }

    return Result;
}
