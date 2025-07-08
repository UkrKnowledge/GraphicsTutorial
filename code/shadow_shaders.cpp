#include "shader_types.h"

cbuffer RenderUniformsBuffer : register(b0)
{
    render_uniforms RenderUniforms;
};

Texture2D Texture : register(t0);
SamplerState BilinearSampler : register(s0);

ps_input ShadowVsMain(vs_input Input)
{
    ps_input Result;
    
    Result.Position = mul(RenderUniforms.ShadowWvpTransform, float4(Input.Position, 1.0f));
    Result.Uv = Input.Uv;
    
    return Result;
}

void ShadowPsMain(ps_input Input)
{
    float ColorAlpha = Texture.Sample(BilinearSampler, Input.Uv).a;
    if (ColorAlpha == 0.0f)
    {
        discard;
    }
}