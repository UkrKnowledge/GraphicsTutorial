#include "shader_types.h"

cbuffer RenderUniformsBuffer : register(b0)
{
    render_uniforms RenderUniforms;
};

cbuffer DirLightUniformsBuffer : register(b1)
{
    dir_light_uniforms DirLightUniforms;
};

StructuredBuffer<point_light> PointLightBuffer : register(t1);
Texture2D ShadowMap : register(t3);

Texture2D Texture : register(t0);
Texture2D NormalTexture : register(t2);
SamplerState BilinearSampler : register(s0);
SamplerState PointSampler : register(s1);

ps_input ModelVsMain(vs_input Input)
{
    ps_input Result;
    
    Result.Position = mul(RenderUniforms.WVPTransform, float4(Input.Position, 1.0f));
    Result.WorldPos = mul(RenderUniforms.WTransform, float4(Input.Position, 1.0f)).xyz;
    Result.ShadowPos = mul(RenderUniforms.ShadowWvpTransform, float4(Input.Position, 1.0f)).xyz;
    Result.Uv = Input.Uv;
    Result.Normal = normalize(mul(RenderUniforms.NormalWTransform, float4(Input.Normal, 0.0f)).xyz);
    Result.Tangent = normalize(mul(RenderUniforms.WTransform, float4(Input.Tangent, 0.0f)).xyz);
    Result.BiTangent = normalize(mul(RenderUniforms.WTransform, float4(Input.BiTangent, 0.0f)).xyz);
    
    return Result;
}

struct ps_output
{
    float4 Color : SV_TARGET0;
};

float3 EvaluatePhong(float3 SurfaceColor, float3 SurfaceNormal, float3 SurfaceWorldPos,
                     float SurfaceShininess, float SurfaceSpecularStrength,
                     float3 LightDirection, float3 LightColor, float LightAmbientIntensity)
{
    float3 NegativeLightDir = -LightDirection;
    float AccumIntensity = LightAmbientIntensity;
    
    // NOTE: Дифузне відбиття
    {
        float DiffuseIntensity = max(0, dot(SurfaceNormal, NegativeLightDir));
        AccumIntensity += DiffuseIntensity;
    }
    
    // NOTE: Дзеркальне відбиття
    float SpecularIntensity = 0;
    {
        float3 CameraDirection = normalize(DirLightUniforms.CameraPos - SurfaceWorldPos);
        float3 HalfVector = normalize(NegativeLightDir + CameraDirection);
        SpecularIntensity = SurfaceSpecularStrength * pow(max(0, dot(HalfVector, SurfaceNormal)), SurfaceShininess);
    }
    
    float3 MixedColor = LightColor * SurfaceColor;
    float3 Result = AccumIntensity * MixedColor + SpecularIntensity * LightColor;
    
    return Result;
}

ps_output ModelPsMain(ps_input Input)
{
    ps_output Result;
    float4 SurfaceColor = Texture.Sample(BilinearSampler, Input.Uv);
    float3 SurfaceNormal;
    {
        float3 AxisNormal = normalize(Input.Normal);
        float3 AxisTangent = normalize(Input.Tangent);
        float3 AxisBiTangent = normalize(Input.BiTangent);
        
        float3x3 Tbn = float3x3(AxisTangent.x, AxisBiTangent.x, AxisNormal.x,
                                AxisTangent.y, AxisBiTangent.y, AxisNormal.y,
                                AxisTangent.z, AxisBiTangent.z, AxisNormal.z);
        
        float3 TextureNormal = NormalTexture.Sample(BilinearSampler, Input.Uv).xyz;
        TextureNormal = 2.0f * TextureNormal - 1.0f;
        
        SurfaceNormal = normalize(mul(Tbn, TextureNormal));
    }
    
    if (SurfaceColor.a == 0.0f)
    {
        discard;
    }
    
    Result.Color.rgb = float3(0, 0, 0);
    
    // NOTE: Освітлюємо пікселя з напрямним світлом
    {
        float LightFactor = 1.0f;
        float2 ShadowUv = Input.ShadowPos.xy * 0.5f + float2(0.5f, 0.5f);
        ShadowUv.y = 1.0f - ShadowUv.y;
        float ShadowDepth = ShadowMap.Sample(PointSampler, ShadowUv).x;
        if (!(Input.ShadowPos.z <= ShadowDepth))
        {
            LightFactor = 0.35f;
        }
        
        Result.Color.rgb = LightFactor * EvaluatePhong(SurfaceColor.rgb, SurfaceNormal, Input.WorldPos,
                                                       RenderUniforms.Shininess, RenderUniforms.SpecularStrength,
                                                       DirLightUniforms.Direction, DirLightUniforms.Color, DirLightUniforms.AmbientIntensity);
    }
    
#if 0
    // NOTE: Освітлюємо пікселя з точковими світлами
    for (int PointLightId = 0; PointLightId < DirLightUniforms.NumPointLights; ++PointLightId)
    {
        point_light PointLight = PointLightBuffer[PointLightId];
        
        // NOTE: Обчислюємо ослаблення світла
        float3 VectorToLight = Input.WorldPos - PointLight.Pos;
        float Radius = length(VectorToLight);
        float3 AttenuatedLight = PointLight.Color / (PointLight.DivisorConstant + Radius * Radius);
        
        VectorToLight /= Radius;
        Result.Color.rgb += EvaluatePhong(SurfaceColor.rgb, SurfaceNormal, Input.WorldPos,
                                          RenderUniforms.Shininess, RenderUniforms.SpecularStrength,
                                          VectorToLight, AttenuatedLight, 0);
    }
#endif
    
    Result.Color.a = SurfaceColor.a;
    
    return Result;
}



