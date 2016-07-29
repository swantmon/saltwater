
#ifndef __INCLUDE_FS_LIGHT_IMAGELIGHT_GLSL__
#define __INCLUDE_FS_LIGHT_IMAGELIGHT_GLSL__

#include "common.glsl"
#include "common_light.glsl"
#include "common_gbuffer.glsl"
#include "common_global.glsl"

// -----------------------------------------------------------------------------
// Input from engine
// -----------------------------------------------------------------------------
layout(std140, binding = 1) uniform UB1
{
    vec4 ps_ConstantBufferData0;
};

layout(std430, binding = 0) buffer BB0
{
    float ps_ExposureHistory[8];
};

layout(binding = 0) uniform sampler2D   ps_GBuffer0;
layout(binding = 1) uniform sampler2D   ps_GBuffer1;
layout(binding = 2) uniform sampler2D   ps_GBuffer2;
layout(binding = 3) uniform sampler2D   ps_Depth;
layout(binding = 4) uniform sampler2D   ps_BRDF;
layout(binding = 5) uniform samplerCube ps_SpecularCubemap;
layout(binding = 6) uniform samplerCube ps_DiffuseCubemap;

// -----------------------------------------------------------------------------
// Easy access
// -----------------------------------------------------------------------------
#define ps_NumberOfMiplevelsSpecularIBL ps_ConstantBufferData0.x
#define ps_ExposureHistoryIndex         ps_ConstantBufferData0.y

// -----------------------------------------------------------------------------
// Input
// -----------------------------------------------------------------------------
layout(location = 2) in vec2 in_UV;

// -----------------------------------------------------------------------------
// Output
// -----------------------------------------------------------------------------
layout(location = 0) out vec4 out_Output;

// -----------------------------------------------------------------------------
// Functions
// -----------------------------------------------------------------------------
vec3 EvaluateDiffuseIBL(in SSurfaceData _Data, in vec3 _WSViewDirection, in float _PreF, in float _NdotV)
{
    vec3 DiffuseDominantN = GetDiffuseDominantDir(_Data.m_WSNormal, _WSViewDirection, _NdotV, _Data.m_Roughness);
    vec3 DiffuseIBL       = textureLod(ps_DiffuseCubemap, DiffuseDominantN, 0).rgb;
    
    DiffuseIBL = mix(DiffuseIBL * 0.3f, DiffuseIBL, _Data.m_AmbientOcclusion);

    return _Data.m_DiffuseAlbedo * DiffuseIBL * _PreF / PI;
}

// -----------------------------------------------------------------------------

vec3 EvaluateSpecularIBL(in SSurfaceData _Data, in vec3 _WSReflectVector, in vec2 _PreDFG, in float _NdotV)
{
    vec3 SpecularDominantR = GetSpecularDominantDir(_Data.m_WSNormal, _WSReflectVector, _Data.m_Roughness);
    
    ivec2 DFGSize = textureSize(ps_BRDF, 0);
    
    // -----------------------------------------------------------------------------
    // Rebuild the function
    // -----------------------------------------------------------------------------
    _NdotV = max(_NdotV, 0.5f / DFGSize.x);
    
    // -----------------------------------------------------------------------------
    // Sample specular cubemap
    // -----------------------------------------------------------------------------
    float LOD         = GetMipLevelByRoughness(_Data.m_Roughness, ps_NumberOfMiplevelsSpecularIBL);
    vec3  SpecularIBL = textureLod(ps_SpecularCubemap, SpecularDominantR, LOD).rgb;
    
    // -----------------------------------------------------------------------------
    // Output
    // -----------------------------------------------------------------------------
    float F90 = clamp(50.0f * dot(_Data.m_SpecularAlbedo, vec3(0.33f)), 0.0f, 1.0f);
    float AO  = GetSpecularOcclusion(_NdotV, _Data.m_AmbientOcclusion, _Data.m_Roughness);
    
    return SpecularIBL * (_Data.m_SpecularAlbedo * _PreDFG.x + F90 * _PreDFG.y) * AO;
}

// -----------------------------------------------------------------------------
// Main
// -----------------------------------------------------------------------------
void main()
{
    // -----------------------------------------------------------------------------
    // Get data
    // -----------------------------------------------------------------------------
    vec4  GBuffer0 = texture(ps_GBuffer0, in_UV);
    vec4  GBuffer1 = texture(ps_GBuffer1, in_UV);
    vec4  GBuffer2 = texture(ps_GBuffer2, in_UV);
    float VSDepth  = texture(ps_Depth   , in_UV).r;

    // -----------------------------------------------------------------------------
    // VS position
    // -----------------------------------------------------------------------------
    vec3 VSPosition = GetViewSpacePositionFromDepth(VSDepth, in_UV, ps_ScreenToView);
    
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
    float AverageExposure = ps_ExposureHistory[uint(ps_ExposureHistoryIndex)];
    
    // -----------------------------------------------------------------------------
    // Compute lighting for sphere lights
    // -----------------------------------------------------------------------------
    vec3  WSViewDirection = normalize(ps_ViewPosition.xyz - Data.m_WSPosition);
    vec3  WSReflectVector = normalize(reflect(-WSViewDirection, Data.m_WSNormal));
    float NdotV           = clamp( dot( Data.m_WSNormal, WSViewDirection ), 0.0, 1.0f);
    
    // -----------------------------------------------------------------------------
    // Get data
    // -----------------------------------------------------------------------------
    vec3 PreDFGF = textureLod(ps_BRDF, vec2(NdotV, Data.m_Roughness), 0).rgb;
    
    vec3 DiffuseIBL  = EvaluateDiffuseIBL(Data, WSViewDirection, PreDFGF.z, NdotV);
    vec3 SpecularIBL = EvaluateSpecularIBL(Data, WSReflectVector, PreDFGF.xy, NdotV);
    
    // -------------------------------------------------------------------------------------
    // Combination of lighting
    // -------------------------------------------------------------------------------------
    vec3 Luminance = DiffuseIBL.rgb + SpecularIBL.rgb;
    
//    AverageExposure = 0.0f;

    // -------------------------------------------------------------------------------------
    // Output
    // -------------------------------------------------------------------------------------
    out_Output = vec4(Luminance * AverageExposure, 1.0f);
}

#endif // __INCLUDE_FS_LIGHT_IMAGELIGHT_GLSL__