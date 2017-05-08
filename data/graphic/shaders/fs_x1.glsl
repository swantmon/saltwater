
#ifndef __INCLUDE_FS_X1_GLSL__
#define __INCLUDE_FS_X1_GLSL__

#include "common.glsl"
#include "common_light.glsl"
#include "common_global.glsl"
#include "common_material.glsl"

// -----------------------------------------------------------------------------
// Definitions
// -----------------------------------------------------------------------------
struct SLightProperties
{
    mat4 ps_LightViewProjection;
    vec4 ps_LightPosition;
    vec4 ps_LightDirection;
    vec4 ps_LightColor;
    vec4 ps_LightSettings;
    uint ps_ExposureHistoryIndex;
    uint ps_LightType;
};

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

layout(std430, binding = 0) readonly buffer BB0
{
    float ps_ExposureHistory[8];
};

layout(std430, binding = 1) readonly buffer BB1
{
    SLightProperties ps_LightProperties[];
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
    vec3 Luminance  = vec3(0.0f);

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
    // Forward pass for each light
    // -----------------------------------------------------------------------------
    for (uint IndexOfLight = 0; IndexOfLight < ps_LightProperties.length(); ++ IndexOfLight)
    {
        SLightProperties LightProb = ps_LightProperties[IndexOfLight];

        if (LightProb.ps_LightType == 1)
        {
            // -----------------------------------------------------------------------------
            // Exposure data
            // -----------------------------------------------------------------------------
            float AverageExposure = ps_ExposureHistory[LightProb.ps_ExposureHistoryIndex];

            // -----------------------------------------------------------------------------
            // Compute lighting for sun light
            // -----------------------------------------------------------------------------
            vec3 WSLightDirection  = -LightProb.ps_LightDirection.xyz;
            vec3 WSViewDirection   = normalize(vec3(0.0f, 0.0f, 10.0f) - Data.m_WSPosition);
            
            float NdotV = dot(Data.m_WSNormal, WSViewDirection);
            
            vec3 ViewMirrorUnitDir = 2.0f * NdotV * Data.m_WSNormal - WSViewDirection;

            // -----------------------------------------------------------------------------
            // Compute sun data
            // -----------------------------------------------------------------------------
            float r = sin(LightProb.ps_LightSettings[0]);
            float d = cos(LightProb.ps_LightSettings[0]);

            float DdotR = dot(WSLightDirection, ViewMirrorUnitDir);
            vec3  S     = ViewMirrorUnitDir - DdotR * WSLightDirection;
            vec3  L     = DdotR < d ? normalize(d * WSLightDirection + normalize(S) * r) : ViewMirrorUnitDir;
            
            // -----------------------------------------------------------------------------
            // Compute attenuation
            // -----------------------------------------------------------------------------
            float Attenuation = 1.0f;
            Attenuation *= Data.m_AmbientOcclusion;
            // Attenuation = GetShadowAtPositionWithPCF(Data.m_WSPosition, ps_LightViewProjection, ps_ShadowTexture);
            
            // -----------------------------------------------------------------------------
            // Apply light luminance
            // -----------------------------------------------------------------------------
            Luminance += BRDF(L, WSViewDirection, Data.m_WSNormal, Data) * clamp(dot(Data.m_WSNormal, L), 0.0f, 1.0f) * LightProb.ps_LightColor.xyz * Attenuation * AverageExposure;
        }
        else if (LightProb.ps_LightType == 2)
        {
            // -----------------------------------------------------------------------------
            // Exposure data
            // -----------------------------------------------------------------------------
            float AverageExposure = ps_ExposureHistory[LightProb.ps_ExposureHistoryIndex];

            // -----------------------------------------------------------------------------
            // Light data
            // -----------------------------------------------------------------------------
            float LightInvSqrAttenuationRadius = LightProb.ps_LightSettings.x;
            float LightAngleScale              = LightProb.ps_LightSettings.y;
            float LightAngleOffset             = LightProb.ps_LightSettings.z;
            float LightHasShadows              = LightProb.ps_LightSettings.w;

            // -----------------------------------------------------------------------------
            // Compute lighting for punctual lights
            // -----------------------------------------------------------------------------
            vec3 UnnormalizedLightVector = LightProb.ps_LightPosition.xyz - Data.m_WSPosition;
            vec3 NormalizedLightVector   = normalize(UnnormalizedLightVector);
            vec3 WSViewDirection         = normalize(vec3(0.0f, 0.0f, 10.0f) - Data.m_WSPosition);

            // -----------------------------------------------------------------------------
            // Compute attenuation
            // -----------------------------------------------------------------------------
            float Attenuation = 1.0f;

            Attenuation *= GetDistanceAttenuation(UnnormalizedLightVector, LightInvSqrAttenuationRadius);
            Attenuation *= GetAngleAttenuation(NormalizedLightVector, -LightProb.ps_LightDirection.xyz, LightAngleScale, LightAngleOffset);

            // -----------------------------------------------------------------------------
            // Shadowing
            // -----------------------------------------------------------------------------
            Attenuation *= Data.m_AmbientOcclusion;
            // Attenuation *= LightHasShadows == 1.0f ? GetShadowAtPosition(Data.m_WSPosition, LightProb.ps_LightViewProjection, ps_Shadowmap) : 1.0f;
            
            // -----------------------------------------------------------------------------
            // Apply light luminance and shading
            // -----------------------------------------------------------------------------
            Luminance += BRDF(NormalizedLightVector, WSViewDirection, Data.m_WSNormal, Data) * clamp(dot(Data.m_WSNormal, NormalizedLightVector), 0.0f, 1.0f) * LightProb.ps_LightColor.xyz * Attenuation * AverageExposure;
        }
    }

    out_Output = vec4(Luminance, 0.0f);
}

#endif // __INCLUDE_FS_X1_GLSL__