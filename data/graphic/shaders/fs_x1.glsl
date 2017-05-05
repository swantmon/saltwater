
#ifndef __INCLUDE_FS_X1_GLSL__
#define __INCLUDE_FS_X1_GLSL__

#include "common.glsl"
#include "common_light.glsl"
#include "common_global.glsl"
#include "common_material.glsl"

// -----------------------------------------------------------------------------
// Input from engine
// -----------------------------------------------------------------------------
layout(std140, binding = 4) uniform UB4
{
    vec4  ps_TilingOffset;
    vec3  ps_Color;
    float ps_Roughness;
    float ps_Reflectance;
    float ps_MetalMask;
};

layout(std140, binding = 5) uniform UB5
{
    mat4  ps_LightViewProjection;
    vec4  ps_LightDirection;
    vec4  ps_LightColor;
    float ps_SunAngularRadius;
    uint  ps_ExposureHistoryIndex;
};

layout(std430, binding = 0) readonly buffer BB0
{
    float ps_ExposureHistory[8];
};

//layout(binding = 0) uniform sampler2DShadow ps_ShadowTexture;
layout(binding = 0) uniform sampler2D PSTextureDiffuse;
layout(binding = 1) uniform sampler2D PSTextureNormal;
layout(binding = 2) uniform sampler2D PSTextureRoughness;
layout(binding = 3) uniform sampler2D PSTextureMetallic;
layout(binding = 4) uniform sampler2D PSTextureAO;

// -----------------------------------------------------------------------------
// Input to fragment from VS
// -----------------------------------------------------------------------------
layout(location = 0) in vec3 in_Position;
layout(location = 1) in vec3 in_Normal;
layout(location = 2) in vec2 in_UV;
layout(location = 3) in mat3 in_WSNormalMatrix;

// -----------------------------------------------------------------------------
// Output to fragment
// -----------------------------------------------------------------------------
layout(location = 0) out vec4 out_Output;

// -----------------------------------------------------------------------------
// Main
// -----------------------------------------------------------------------------
void main(void)
{    
    vec2  UV        = in_UV * ps_TilingOffset.xy + ps_TilingOffset.zw;
    vec3  Color     = ps_Color.xyz;
    vec3  WSNormal  = in_Normal;
    float Roughness = ps_Roughness;
    float MetalMask = ps_MetalMask;
    float AO        = 1.0f;

#ifdef USE_TEX_DIFFUSE
    Color *= texture(PSTextureDiffuse, UV).rgb;
#endif // USE_TEX_DIFFUSE

#ifdef USE_TEX_NORMAL
    WSNormal = in_WSNormalMatrix * (texture(PSTextureNormal, UV).rgb * 2.0f - 1.0f);
#endif // USE_TEX_NORMAL

#ifdef USE_TEX_ROUGHNESS
    Roughness *= texture(PSTextureRoughness, UV).r;
#endif // USE_TEX_ROUGHNESS

#ifdef USE_TEX_METALLIC
    MetalMask *= texture(PSTextureMetallic, UV).r;
#endif // USE_TEX_METALLIC

    // -----------------------------------------------------------------------------
    // Surface data
    // -----------------------------------------------------------------------------
    SSurfaceData Data;

    Data.m_VSDepth          = 1.0f;
    Data.m_WSPosition       = in_Position;
    Data.m_WSNormal         = -WSNormal;
    Data.m_DiffuseAlbedo    = Color * (1.0f - MetalMask);
    Data.m_SpecularAlbedo   = Color * MetalMask + 0.16f * ps_Reflectance * ps_Reflectance * (1.0f - MetalMask);
    Data.m_Roughness        = clamp(Roughness, 0.0f, 1.0f);
    Data.m_AmbientOcclusion = AO;

    // -----------------------------------------------------------------------------
    // Compute lighting for sun light
    // -----------------------------------------------------------------------------
    vec3 WSLightDirection  = -ps_LightDirection.xyz;
    vec3 WSViewDirection   = normalize(vec3(0,0,10) - Data.m_WSPosition);
    
    float NdotV = dot(Data.m_WSNormal, WSViewDirection);
    
    vec3 ViewMirrorUnitDir = 2.0f * NdotV * Data.m_WSNormal - WSViewDirection;

    // -----------------------------------------------------------------------------
    // Compute sun data
    // -----------------------------------------------------------------------------
    float r = sin(ps_SunAngularRadius);
    float d = cos(ps_SunAngularRadius);

    float DdotR = dot(WSLightDirection, ViewMirrorUnitDir);
    vec3  S     = ViewMirrorUnitDir - DdotR * WSLightDirection;
    vec3  L     = DdotR < d ? normalize(d * WSLightDirection + normalize(S) * r) : ViewMirrorUnitDir;
    
    // float Shadow = GetShadowAtPositionWithPCF(Data.m_WSPosition, ps_LightViewProjection, ps_ShadowTexture);
    float Shadow = 1.0f;
    
    // -----------------------------------------------------------------------------
    // Apply light luminance
    // -----------------------------------------------------------------------------
    vec3 Luminance = BRDF(L, WSViewDirection, Data.m_WSNormal, Data) * clamp(dot(Data.m_WSNormal, L), 0.0f, 1.0f) * ps_LightColor.xyz * Data.m_AmbientOcclusion * Shadow;

    out_Output = vec4(Luminance, 0.0f);
}

#endif // __INCLUDE_FS_X1_GLSL__