
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

// -----------------------------------------------------------------------------
// Input to fragment from VS
// -----------------------------------------------------------------------------
layout(location = 0) in vec3 in_Position;
layout(location = 1) in vec3 in_Normal;
layout(location = 2) in vec2 in_UV;

// -----------------------------------------------------------------------------
// Output to fragment
// -----------------------------------------------------------------------------
layout(location = 0) out vec4 out_Output;

// -----------------------------------------------------------------------------
// Main
// -----------------------------------------------------------------------------
void main(void)
{    
    vec2 UV    = in_UV * ps_TilingOffset.xy + ps_TilingOffset.zw;
    vec3 Color = ps_Color.xyz;

#ifdef USE_TEX_DIFFUSE
    Color *= texture(PSTextureDiffuse, UV).rgb;
#endif // USE_TEX_DIFFUSE

    //out_Output = vec4(Color * 10000.0f, 0.0f); return;
    //out_Output = vec4(Color * ps_LightColor.xyz, 0.0f); return;
    out_Output = vec4(Color * ps_LightColor.xyz * clamp(dot(in_Normal, ps_LightDirection.xyz), 0.0f, 1.0f), 0.0f); return;





    // -----------------------------------------------------------------------------
    // Surface data
    // -----------------------------------------------------------------------------
    SSurfaceData Data;

    Data.m_VSDepth          = 1.0f;
    Data.m_WSPosition       = in_Position;
    Data.m_WSNormal         = -in_Normal;
    Data.m_DiffuseAlbedo    = Color;
    Data.m_SpecularAlbedo   = Color;
    Data.m_Roughness        = ps_Roughness;
    Data.m_AmbientOcclusion = 1.0f;



    // -----------------------------------------------------------------------------
    // Exposure data
    // -----------------------------------------------------------------------------
    float AverageExposure = ps_ExposureHistory[ps_ExposureHistoryIndex];
    
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

//    AverageExposure = 0.0f;

    out_Output = vec4(Luminance * 10000, 0.0f);
}

#endif // __INCLUDE_FS_X1_GLSL__