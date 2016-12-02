
#ifndef __INCLUDE_FS_LIGHT_PUNCTUALLIGHT_GLSL__
#define __INCLUDE_FS_LIGHT_PUNCTUALLIGHT_GLSL__

#include "common.glsl"
#include "common_global.glsl"
#include "common_light.glsl"
#include "common_gbuffer.glsl"

// -----------------------------------------------------------------------------
// Defines
// -----------------------------------------------------------------------------
#define GLOSSY_NORMALIZATION_CONSTANT 0.1f
#define RSM_WIDTH                     256 / 16

// -----------------------------------------------------------------------------
// Input from engine
// -----------------------------------------------------------------------------
layout(row_major, std140, binding = 1) uniform UB1
{
    vec4 m_RSMSettings; // x = TexelSizeWidth, y = TexelOffsetHeight z = Light intensity, w = unused
    uint ps_ExposureHistoryIndex;
};

layout(std430, binding = 0) buffer UExposureHistoryBuffer
{
    float ps_ExposureHistory[8];
};

layout(binding = 0) uniform sampler2D ps_GBuffer0;
layout(binding = 1) uniform sampler2D ps_GBuffer1;
layout(binding = 2) uniform sampler2D ps_GBuffer2;
layout(binding = 3) uniform sampler2D ps_DepthTexture;
layout(binding = 4) uniform sampler2D ps_ShadowmapPosition;
layout(binding = 5) uniform sampler2D ps_ShadowmapNormal;
layout(binding = 6) uniform sampler2D ps_ShadowmapFlux;
layout(binding = 7) uniform sampler2D ps_ShadowmapDepth;

// -----------------------------------------------------------------------------
// Input from prev. stage
// -----------------------------------------------------------------------------
layout (location = 2) in vec2 in_TexCoord;

// -----------------------------------------------------------------------------
// Output
// -----------------------------------------------------------------------------
layout (location = 0) out vec4 out_Output;

// -----------------------------------------------------------------------------
// Main
// -----------------------------------------------------------------------------
void main()
{
    // -----------------------------------------------------------------------------
    // Get data
    // -----------------------------------------------------------------------------
    vec4  GBuffer0 = texture(ps_GBuffer0    , in_TexCoord);
    vec4  GBuffer1 = texture(ps_GBuffer1    , in_TexCoord);
    vec4  GBuffer2 = texture(ps_GBuffer2    , in_TexCoord);
    float VSDepth  = texture(ps_DepthTexture, in_TexCoord).r;

    // -----------------------------------------------------------------------------
    // VS position
    // -----------------------------------------------------------------------------
    vec3 VSPosition = GetViewSpacePositionFromDepth(VSDepth, in_TexCoord, g_ScreenToView);
    
    // -----------------------------------------------------------------------------
    // WS position
    // -----------------------------------------------------------------------------
    vec3 WSPosition = (g_ViewToWorld * vec4(VSPosition, 1.0f)).xyz;

    // -----------------------------------------------------------------------------
    // Surface data
    // -----------------------------------------------------------------------------
    SSurfaceData Data;

    UnpackGBuffer(GBuffer0, GBuffer1, GBuffer2, WSPosition.xyz, VSDepth, Data);

    // -----------------------------------------------------------------------------
    // Exposure data
    // -----------------------------------------------------------------------------
    float AverageExposure = ps_ExposureHistory[ps_ExposureHistoryIndex];
    
    // -----------------------------------------------------------------------------
    // Compute indirect lieghting in screen space for given reflective shadow map
    // -----------------------------------------------------------------------------
    vec3 DiffuseColor  = vec3(0.0);
    vec3 SpecularColor = vec3(0.0);
    
    vec3 WSViewDirection = normalize(g_ViewPosition.xyz - Data.m_WSPosition);
    
    float IndirectLightIntensity = m_RSMSettings.z;

    float SpecularExponent = 1.0f;
    
    // -----------------------------------------------------------------------------
    // Create an spectrum of light sending from the current world-space position
    // -----------------------------------------------------------------------------
    for (int IndexOfSample = 0; IndexOfSample < RSM_WIDTH; ++ IndexOfSample)
    {
        // -----------------------------------------------------------------------------
        // Get necessary data
        // -----------------------------------------------------------------------------
        vec2 TexCoordOffset = vec2(m_RSMSettings.x * float(IndexOfSample), m_RSMSettings.y);
        
        vec3  LightNormal     = texture(ps_ShadowmapNormal  , TexCoordOffset).rgb;
        vec3  LightFlux       = texture(ps_ShadowmapFlux    , TexCoordOffset).rgb;
        vec3  WSLightPosition = texture(ps_ShadowmapPosition, TexCoordOffset).rgb;
        float LightDepth      = texture(ps_ShadowmapDepth   , TexCoordOffset).r;
        
        // -----------------------------------------------------------------------------
        // Build reflection vector
        // -----------------------------------------------------------------------------
        vec3 Reflection = WSLightPosition.xyz - WSPosition.xyz;
        
        float l2    = dot(Reflection, Reflection);
        
        Reflection *= inversesqrt(l2);
        l2          = sqrt(l2);
        
        float lR = (LightDepth / (LightDepth + l2));
        
        // -----------------------------------------------------------------------------
        // Build normal angles
        // -----------------------------------------------------------------------------
        float CosThetaI = clamp(dot(LightNormal, -Reflection), 0.0f, 1.0f);
        float CosThetaJ = clamp(dot(Data.m_WSNormal, Reflection), 0.0f, 1.0f);
        
        float Fij = CosThetaI * CosThetaJ * lR;
        
        // -----------------------------------------------------------------------------
        // Compute flux/color
        // -----------------------------------------------------------------------------
        DiffuseColor += LightFlux * Fij;
        
        // -----------------------------------------------------------------------------
        // Compute specular
        // -----------------------------------------------------------------------------
        CosThetaJ = pow( clamp(dot(WSViewDirection, reflect( -Reflection, Data.m_WSNormal.xyz )), 0.0f, 1.0f), SpecularExponent);
        
        Fij = CosThetaI * CosThetaJ * lR;
        
        SpecularColor += LightFlux * Fij * GLOSSY_NORMALIZATION_CONSTANT * (SpecularExponent + 1.0f);
    }
    
    // -----------------------------------------------------------------------------
    // Output final color
    // -----------------------------------------------------------------------------
    vec3 Diffuse  = Data.m_DiffuseAlbedo * DiffuseColor * Data.m_AmbientOcclusion;
    
    vec3 Specular = Data.m_SpecularAlbedo * SpecularColor;
    
    // -----------------------------------------------------------------------------
    // Output final color
    // -----------------------------------------------------------------------------
    out_Output = vec4((Diffuse + Specular) * IndirectLightIntensity * AverageExposure, 0.0f);
}

#endif // __INCLUDE_FS_LIGHT_PUNCTUALLIGHT_GLSL__