
#ifndef __INCLUDE_FS_LIGHT_DISKLIGHT_GLSL__
#define __INCLUDE_FS_LIGHT_DISKLIGHT_GLSL__

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
layout(row_major, std140, binding = 0) uniform UB0
{
    mat4 m_InverseCameraProjection;
    mat4 m_InverseCameraView;
    vec4 m_CameraPosition;
    vec4 m_InvertedScreenSize;
    uint m_ExposureHistoryIndex;
};

layout(row_major, std140, binding = 1) uniform UB1
{
    vec4  m_LightPosition;
    vec4  m_LightDirection;
    vec4  m_LightColor;
    vec4  m_LightSettings; // InvSqrAttenuationRadius, Radius, AngleScale, AngleOffset
};

layout(std430, binding = 0) buffer UExposureHistoryBuffer
{
    float m_ExposureHistory[8];
};

uniform sampler2D   PSGBufferTexture0;
uniform sampler2D   PSGBufferTexture1;
uniform sampler2D   PSGBufferTexture2;
uniform sampler2D   PSDepthTexture;

// -----------------------------------------------------------------------------
// Output to light accumulation target
// -----------------------------------------------------------------------------
layout (location = 0) out vec4 PSOutput;

// -----------------------------------------------------------------------------
// Main
// -----------------------------------------------------------------------------
void main()
{
    // -----------------------------------------------------------------------------
    // Define tex coords from system input
    // -----------------------------------------------------------------------------
    vec2 TexCoord = vec2(gl_FragCoord.x * m_InvertedScreenSize.x, gl_FragCoord.y * m_InvertedScreenSize.y);
    
    // -----------------------------------------------------------------------------
    // Get data
    // -----------------------------------------------------------------------------
    vec4  GBuffer0 = texture(PSGBufferTexture0, TexCoord);
    vec4  GBuffer1 = texture(PSGBufferTexture1, TexCoord);
    vec4  GBuffer2 = texture(PSGBufferTexture2, TexCoord);
    float VSDepth  = texture(PSDepthTexture   , TexCoord).r;

    // -----------------------------------------------------------------------------
    // VS position
    // -----------------------------------------------------------------------------
    vec3 VSPosition = GetViewSpacePositionFromDepth(VSDepth, TexCoord, m_InverseCameraProjection);
    
    // -----------------------------------------------------------------------------
    // WS position
    // -----------------------------------------------------------------------------
    vec3 WSPosition = (m_InverseCameraView * vec4(VSPosition, 1.0f)).xyz;

    // -----------------------------------------------------------------------------
    // Surface data
    // -----------------------------------------------------------------------------
    SSurfaceData Data;

    UnpackGBuffer(GBuffer0, GBuffer1, GBuffer2, WSPosition.xyz, VSDepth, Data);

    // -----------------------------------------------------------------------------
    // Exposure data
    // -----------------------------------------------------------------------------
    float AverageExposure = m_ExposureHistory[m_ExposureHistoryIndex];
    
    // -----------------------------------------------------------------------------
    // Light data
    // -----------------------------------------------------------------------------
    float LightInvSqrAttenuationRadius = m_LightSettings.x;
    float LightRadius                  = m_LightSettings.y;
    float LightAngleScale              = m_LightSettings.z;
    float LightAngleOffset             = m_LightSettings.w;
    
    // -----------------------------------------------------------------------------
    // Compute lighting for sphere lights
    // -----------------------------------------------------------------------------
    vec3 UnnormalizedLightVector = m_LightPosition.xyz - Data.m_WSPosition;
    vec3 WSLightDirection        = normalize(UnnormalizedLightVector);
    vec3 WSViewDirection         = normalize(m_CameraPosition.xyz - Data.m_WSPosition);
    
    float SqrDistance = dot(UnnormalizedLightVector, UnnormalizedLightVector);

    // -----------------------------------------------------------------------------
    // Compute attenuation
    // -----------------------------------------------------------------------------
    float Attenuation = 1.0f; 

    Attenuation *= GetAngleAttenuation(WSLightDirection, m_LightDirection.xyz, LightAngleScale, LightAngleOffset);
    Attenuation *= Data.m_AmbientOcclusion;
    
    // -----------------------------------------------------------------------------
    // Compute sphere illuminance
    // -----------------------------------------------------------------------------
    float CosTheta       = dot(Data.m_WSNormal, WSLightDirection);
    float SqrLightRadius = LightRadius * LightRadius;
    float SinSigmaSqr    = SqrLightRadius / (SqrLightRadius + max(SqrLightRadius, SqrDistance));
    
    float Illuminance = PI * SqrLightRadius * clamp(CosTheta, 0.0f, 1.0f) * clamp(dot(m_LightDirection.xyz, WSLightDirection), 0.0f, 1.0f) / SqrDistance;

    if (SqrDistance < 100.0f * SqrLightRadius)
    {
         Illuminance = GetIlluminanceSphereOrDisk(CosTheta, SinSigmaSqr) * clamp(dot(m_LightDirection.xyz, WSLightDirection), 0.0f, 1.0f);
    }

    Illuminance *= Attenuation;
    
    // -----------------------------------------------------------------------------
    // Apply light luminance and shading
    // -----------------------------------------------------------------------------
    vec3 Luminance = BRDF(WSLightDirection, WSViewDirection, Data.m_WSNormal, Data) * clamp(dot(Data.m_WSNormal, WSLightDirection), 0.0f, 1.0f) * m_LightColor.xyz * Illuminance;
    
//    AverageExposure = 0.0f;

    PSOutput = vec4(Luminance * AverageExposure, 0.0f);
}

#endif // __INCLUDE_FS_LIGHT_DISKLIGHT_GLSL__