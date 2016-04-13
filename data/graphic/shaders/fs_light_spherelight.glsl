
#ifndef __INCLUDE_FS_LIGHT_SPHERELIGHT_GLSL__
#define __INCLUDE_FS_LIGHT_SPHERELIGHT_GLSL__

#include "common.glsl"
#include "common_light.glsl"
#include "common_gbuffer.glsl"

// -----------------------------------------------------------------------------
// Input from system
// -----------------------------------------------------------------------------
in vec4 gl_FragCoord;

// -----------------------------------------------------------------------------
// Input from engine
// -----------------------------------------------------------------------------
layout(row_major, std140, binding = 0) uniform UCameraProperties
{
    mat4 ps_InverseCameraProjection;
    mat4 ps_InverseCameraView;
    vec4 ps_CameraPosition;
    vec4 ps_InvertedScreenSize;
    uint ps_ExposureHistoryIndex;
};

layout(row_major, std140, binding = 1) uniform USphereLightProperties
{
    vec4  ps_LightPosition;
    vec4  ps_LightColor;
    vec4  ps_LightSettings; // InvSqrAttenuationRadius, Radius
};

layout(std430, binding = 0) buffer UExposureHistoryBuffer
{
    float ps_ExposureHistory[8];
};

layout(binding = 0) uniform sampler2D ps_GBuffer0;
layout(binding = 1) uniform sampler2D ps_GBuffer1;
layout(binding = 2) uniform sampler2D ps_GBuffer2;
layout(binding = 3) uniform sampler2D ps_Depth;

// -----------------------------------------------------------------------------
// Output to light accumulation target
// -----------------------------------------------------------------------------
layout (location = 0) out vec4 out_Output;

// -----------------------------------------------------------------------------
// Main
// -----------------------------------------------------------------------------
void main()
{
    // -----------------------------------------------------------------------------
    // Define tex coords from system input
    // -----------------------------------------------------------------------------
    vec2 TexCoord = vec2(gl_FragCoord.x * ps_InvertedScreenSize.x, gl_FragCoord.y * ps_InvertedScreenSize.y);
    
    // -----------------------------------------------------------------------------
    // Get data
    // -----------------------------------------------------------------------------
    vec4  GBuffer0 = texture(ps_GBuffer0, TexCoord);
    vec4  GBuffer1 = texture(ps_GBuffer1, TexCoord);
    vec4  GBuffer2 = texture(ps_GBuffer2, TexCoord);
    float VSDepth  = texture(ps_Depth   , TexCoord).r;

    // -----------------------------------------------------------------------------
    // VS position
    // -----------------------------------------------------------------------------
    vec3 VSPosition = GetViewSpacePositionFromDepth(VSDepth, TexCoord, ps_InverseCameraProjection);
    
    // -----------------------------------------------------------------------------
    // WS position
    // -----------------------------------------------------------------------------
    vec3 WSPosition = (ps_InverseCameraView * vec4(VSPosition, 1.0f)).xyz;

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
    // Light data
    // -----------------------------------------------------------------------------
    float LightInvSqrAttenuationRadius = ps_LightSettings.x;
    float LightRadius                  = ps_LightSettings.y;
    
    // -----------------------------------------------------------------------------
    // Compute lighting for sphere lights
    // -----------------------------------------------------------------------------
    vec3 UnnormalizedLightVector = ps_LightPosition.xyz - Data.m_WSPosition;
    vec3 WSLightDirection        = normalize(UnnormalizedLightVector);
    vec3 WSViewDirection         = normalize(ps_CameraPosition.xyz - Data.m_WSPosition);
    
    float SqrDistance = dot(UnnormalizedLightVector, UnnormalizedLightVector);

    // -----------------------------------------------------------------------------
    // Compute attenuation
    // -----------------------------------------------------------------------------
    float Attenuation = 1.0f; 

    Attenuation *= GetDistanceAttenuation(UnnormalizedLightVector, LightInvSqrAttenuationRadius);
    Attenuation *= Data.m_AmbientOcclusion;
    
    // -----------------------------------------------------------------------------
    // Compute sphere illuminance
    // -----------------------------------------------------------------------------
    float CosTheta       = clamp(dot(Data.m_WSNormal, WSLightDirection), -0.999f, 0.999f);
    float SqrLightRadius = LightRadius * LightRadius;
    float SinSigmaSqr    = min(SqrLightRadius / SqrDistance, 0.9999f);
    
    float Illuminance = PI * SqrLightRadius * clamp(CosTheta, 0.0f, 1.0f) / SqrDistance;

    if (SqrDistance < 100.0f * SqrLightRadius)
    {
        Illuminance = GetIlluminanceSphereOrDisk(CosTheta, SinSigmaSqr);
    }

    Illuminance *= Attenuation;
    
    // -----------------------------------------------------------------------------
    // Apply light luminance and shading
    // -----------------------------------------------------------------------------
    vec3 Luminance = BRDF(WSLightDirection, WSViewDirection, Data.m_WSNormal, Data) * clamp(dot(Data.m_WSNormal, WSLightDirection), 0.0f, 1.0f) * ps_LightColor.xyz * Illuminance;
    
//    AverageExposure = 0.0f;

    out_Output = vec4(Luminance * AverageExposure, 0.0f);
}

#endif // __INCLUDE_FS_LIGHT_SPHERELIGHT_GLSL__