
#ifndef __INCLUDE_FS_LIGHT_SUNLIGHT_GLSL__
#define __INCLUDE_FS_LIGHT_SUNLIGHT_GLSL__

#include "common.glsl"
#include "common_light.glsl"
#include "common_gbuffer.glsl"
#include "common_global.glsl"

// -----------------------------------------------------------------------------
// Input from engine
// -----------------------------------------------------------------------------
layout(row_major, std140, binding = 1) uniform UB1
{
    mat4   ps_LightViewProjection;
    vec4   ps_LightDirection;
    vec4   ps_LightColor;
    float  ps_SunAngularRadius;
    uint   ps_ExposureHistoryIndex;
};

layout(std430, binding = 0) readonly buffer BB0
{
    float ps_ExposureHistory[8];
};

layout(binding = 0) uniform sampler2D ps_GBuffer0;
layout(binding = 1) uniform sampler2D ps_GBuffer1;
layout(binding = 2) uniform sampler2D ps_GBuffer2;
layout(binding = 3) uniform sampler2D ps_DepthTexture;
layout(binding = 4) uniform sampler2D ps_ShadowTexture;

// -----------------------------------------------------------------------------
// Input
// -----------------------------------------------------------------------------
layout(location = 2) in vec2 in_TexCoord;

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
    vec3 VSPosition = GetViewSpacePositionFromDepth(VSDepth, in_TexCoord, ps_ScreenToView);
    
    // -----------------------------------------------------------------------------
    // WS position
    // -----------------------------------------------------------------------------
    vec3 WSPosition = (ps_ViewToWorld * vec4(VSPosition, 1.0f)).xyz;

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
    // Compute lighting for sun light
    // -----------------------------------------------------------------------------
    vec3 WSLightDirection  = -ps_LightDirection.xyz;
    vec3 WSViewDirection   = normalize(ps_ViewPosition.xyz - Data.m_WSPosition);
    
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
    
    float Shadow = GetShadowAtPositionWithPCF(Data.m_WSPosition, ps_LightViewProjection, ps_ShadowTexture);
    
    // -----------------------------------------------------------------------------
    // Apply light luminance
    // -----------------------------------------------------------------------------
    vec3 Luminance = BRDF(L, WSViewDirection, Data.m_WSNormal, Data) * clamp(dot(Data.m_WSNormal, L), 0.0f, 1.0f) * ps_LightColor.xyz * Data.m_AmbientOcclusion * Shadow;

//    AverageExposure = 0.0f;

    out_Output = vec4(Luminance * AverageExposure, 0.0f);
}

#endif // __INCLUDE_FS_LIGHT_SUNLIGHT_GLSL__