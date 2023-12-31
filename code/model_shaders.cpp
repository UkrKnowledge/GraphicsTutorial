
cbuffer RenderUniforms : register(b0)
{
    float4x4 WVPTransform;
    float4x4 WTransform;
    float4x4 NormalWTransform;
    float Shininess;
    float SpecularStrength;
};

cbuffer LightUniforms : register(b1)
{
    float3 LightColor;
    float LightAmbientIntensity;
    float3 LightDirection;
    float3 CameraPos;
};

Texture2D Texture : register(t0);
SamplerState BilinearSampler : register(s0);

struct vs_input
{
    float3 Position : POSITION;
    float2 Uv : TEXCOORD0;
    float3 Normal : NORMAL;
};

struct ps_input
{
    float4 Position : SV_POSITION;
    float3 WorldPos : POSITION;
    float2 Uv : TEXCOORD0;
    float3 Normal : NORMAL;
};

ps_input ModelVsMain(vs_input Input)
{
    ps_input Result;

    Result.Position = mul(WVPTransform, float4(Input.Position, 1.0f));
    Result.WorldPos = mul(WTransform, float4(Input.Position, 1.0f)).xyz;
    Result.Uv = Input.Uv;
    Result.Normal = normalize(mul(NormalWTransform, float4(Input.Normal, 0.0f)).xyz);

    return Result;
}

struct ps_output
{
    float4 Color : SV_TARGET0;
};

ps_output ModelPsMain(ps_input Input)
{
    ps_output Result;
    float4 SurfaceColor = Texture.Sample(BilinearSampler, Input.Uv);
    float3 SurfaceNormal = normalize(Input.Normal);
    if (SurfaceColor.a == 0.0f)
    {
        discard;
    }

    float3 NegativeLightDir = -LightDirection;
    float AccumIntensity = LightAmbientIntensity;
    // NOTE: Дифузне відбиття
    {
        float DiffuseIntensity = max(0, dot(SurfaceNormal, NegativeLightDir));
        AccumIntensity += DiffuseIntensity;
    }

    // NOTE: Дзеркальне відбиття
    {
        float3 CameraDirection = normalize(CameraPos - Input.WorldPos);
        float3 ReflectionVector = -(NegativeLightDir - 2 * dot(NegativeLightDir, SurfaceNormal) * SurfaceNormal);
        float SpecularIntensity = SpecularStrength * pow(max(0, dot(ReflectionVector, CameraDirection)), Shininess);
        AccumIntensity += SpecularIntensity;
    }
    
    float3 MixedColor = LightColor * SurfaceColor.rgb;
    Result.Color = float4(AccumIntensity * MixedColor, SurfaceColor.a);
    
    return Result;
}
