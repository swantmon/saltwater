
#ifndef __INCLUDE_FS_CAUSTIC_EMISSION_GLSL__
#define __INCLUDE_FS_CAUSTIC_EMISSION_GLSL__

#include "common.glsl"
#include "common_light.glsl"
#include "common_gbuffer.glsl"
#include "common_material.glsl"

// -----------------------------------------------------------------------------
// Input from engine
// -----------------------------------------------------------------------------
layout(std140, binding = 2) uniform UB2
{
    vec4 ps_LightPosition;
    vec4 ps_LightDirection;
    vec4 ps_LightColor;
    vec4 ps_LightSettings; // InvSqrAttenuationRadius, AngleScale, AngleOffset, Unused
};

layout(std140, binding = 3) uniform UB3
{
    vec4  ps_TilingOffset;
    vec4  ps_Color;
    float ps_Roughness;
    float ps_Reflectance;
    float ps_MetalMask;
};

layout(binding =  0) uniform sampler2D ps_DiffuseTexture;
layout(binding =  1) uniform sampler2D ps_NormalTexture;
layout(binding =  2) uniform sampler2D ps_RougnessTexture;
layout(binding =  3) uniform sampler2D ps_Metaltexture;
layout(binding =  4) uniform sampler2D ps_AOTexture;

// -----------------------------------------------------------------------------
// Input to fragment from previous stage
// -----------------------------------------------------------------------------
layout(location = 0) in vec3 in_Position;
layout(location = 1) in vec3 in_Normal;
layout(location = 2) in vec2 in_UV;
layout(location = 3) in mat3 in_WSNormalMatrix;

// -----------------------------------------------------------------------------
// Output to fragment
// -----------------------------------------------------------------------------
layout(location = 0) out vec4 out_Flux;
layout(location = 1) out vec4 out_PhotonLocation;

// -----------------------------------------------------------------------------
// Function
// -----------------------------------------------------------------------------
void main(void)
{
    vec2  UV        = in_UV * ps_TilingOffset.xy + ps_TilingOffset.zw;
    vec3  Color     = ps_Color.xyz;
    vec3  WSNormal  = in_Normal;
    float Roughness = ps_Roughness;
    float MetalMask = ps_MetalMask;
    float AO        = 1.0f;
    float Alpha     = ps_Color.w;
    vec3 Luminance  = vec3(0.0f);

#ifdef USE_TEX_DIFFUSE
    Color *= texture(ps_DiffuseTexture, UV).rgb;
#endif // USE_TEX_DIFFUSE

#ifdef USE_TEX_NORMAL
    WSNormal = in_WSNormalMatrix * (texture(ps_NormalTexture, UV).rgb * 2.0f - 1.0f);
#endif // USE_TEX_NORMAL

#ifdef USE_TEX_ROUGHNESS
    Roughness *= texture(ps_RougnessTexture, UV).r;
#endif // USE_TEX_ROUGHNESS

#ifdef USE_TEX_METALLIC
    MetalMask *= texture(ps_Metaltexture, UV).r;
#endif // USE_TEX_METALLIC

#ifdef USE_TEX_AO
    AO *= texture(ps_AOTexture, UV).r;
#endif // USE_TEX_AO

#ifdef USE_TEX_ALPHA
    Alpha *= texture(ps_AlphaTexture, UV).r;
#endif // USE_TEX_ALPHA

    // -----------------------------------------------------------------------------
    // Surface data
    // -----------------------------------------------------------------------------
    SGBuffer GBuffer;

    PackGBuffer(Color, WSNormal, Roughness, vec3(ps_Reflectance), MetalMask, AO, GBuffer);

    SSurfaceData Data;

    UnpackGBuffer(GBuffer.m_Color0, GBuffer.m_Color1, GBuffer.m_Color2, in_Position.xyz, gl_FragCoord.z, Data);

    // -----------------------------------------------------------------------------
    // Light data
    // -----------------------------------------------------------------------------
    float LightInvSqrAttenuationRadius = ps_LightSettings.x;
    float LightAngleScale              = ps_LightSettings.y;
    float LightAngleOffset             = ps_LightSettings.z;
    float LightHasShadows              = ps_LightSettings.w;

    // -----------------------------------------------------------------------------
    // Compute lighting for punctual lights
    // -----------------------------------------------------------------------------
    vec3 UnnormalizedLightVector = ps_LightPosition.xyz - Data.m_WSPosition;
    vec3 NormalizedLightVector   = normalize(UnnormalizedLightVector);
    vec3 WSViewDirection         = normalize(Data.m_WSPosition - ps_LightPosition.xyz);

    // -----------------------------------------------------------------------------
    // Compute attenuation
    // -----------------------------------------------------------------------------
    float Attenuation = 1.0f;

    Attenuation *= GetDistanceAttenuation(UnnormalizedLightVector, LightInvSqrAttenuationRadius);
    Attenuation *= GetAngleAttenuation(NormalizedLightVector, -ps_LightDirection.xyz, LightAngleScale, LightAngleOffset);

    // -----------------------------------------------------------------------------
    // Shadowing
    // -----------------------------------------------------------------------------
    Attenuation *= Data.m_AmbientOcclusion;
    
    // -----------------------------------------------------------------------------
    // Apply light luminance and shading
    // -----------------------------------------------------------------------------
    Luminance += BRDF(NormalizedLightVector, WSViewDirection, Data.m_WSNormal, Data) * clamp(dot(Data.m_WSNormal, NormalizedLightVector), 0.0f, 1.0f) * ps_LightColor.xyz * Attenuation;

    out_PhotonLocation = vec4(in_Position, 1.0f);
    out_Flux           = vec4(Luminance, 1.0f);
}

#endif // __INCLUDE_FS_CAUSTIC_EMISSION_GLSL__