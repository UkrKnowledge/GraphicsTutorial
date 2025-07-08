/* date = December 15th 2024 3:58 pm */

#ifndef SHADER_TYPES_H
#define SHADER_TYPES_H

struct render_uniforms
{
    float4x4 WVPTransform;
    float4x4 WTransform;
    float4x4 NormalWTransform;
    float4x4 ShadowWvpTransform;
    float4x4 ShadowVpTransform;
    float Shininess;
    float SpecularStrength;
};

struct dir_light_uniforms
{
    float3 Color;
    float AmbientIntensity;
    float3 Direction;
    uint NumPointLights;
    float3 CameraPos;
};

struct point_light
{
    float3 Pos;
    float DivisorConstant;
    float3 Color;
};

struct vs_input
{
    float3 Position : POSITION;
    float2 Uv : TEXCOORD0;
    float3 Normal : NORMAL;
    float3 Tangent : TANGENT;
    float3 BiTangent : BITANGENT;
};

struct ps_input
{
    float4 Position : SV_POSITION;
    float3 WorldPos : POSITION0;
    float3 ShadowPos : POSITION1;
    float2 Uv : TEXCOORD0;
    float3 Normal : NORMAL;
    float3 Tangent : TANGENT;
    float3 BiTangent : BITANGENT;
};

#endif //SHADER_TYPES_H
