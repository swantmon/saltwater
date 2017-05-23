
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
layout(binding = 7) uniform samplerCube ps_ShadowCubemap;

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
    vec3 VSPosition = GetViewSpacePositionFromDepth(VSDepth, in_UV, g_ScreenToView);
    
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
    float AverageExposure = ps_ExposureHistory[uint(ps_ExposureHistoryIndex)];
    
    // -----------------------------------------------------------------------------
    // Compute lighting for sphere lights
    // -----------------------------------------------------------------------------
    vec3  WSViewDirection = normalize(Data.m_WSPosition - g_ViewPosition.xyz);
    vec3  WSReflectVector = normalize(reflect(WSViewDirection, Data.m_WSNormal));
    float NdotV           = clamp( dot( Data.m_WSNormal, -WSViewDirection ), 0.0, 1.0f);

    // -----------------------------------------------------------------------------
    // Rebuild the function
    // -----------------------------------------------------------------------------
    ivec2 DFGSize = textureSize(ps_BRDF, 0);

    float ClampNdotV = max(NdotV, 0.5f / DFGSize.x);
    
    // -----------------------------------------------------------------------------
    // Get data
    // -----------------------------------------------------------------------------
    vec3 PreDFGF = textureLod(ps_BRDF, vec2(NdotV, Data.m_Roughness), 0).rgb;
    
    vec3 DiffuseIBL  = EvaluateDiffuseIBL(ps_DiffuseCubemap, Data, WSViewDirection, PreDFGF.z, NdotV);
    vec3 SpecularIBL = EvaluateSpecularIBL(ps_SpecularCubemap, Data, WSReflectVector, PreDFGF.xy, ClampNdotV, ps_NumberOfMiplevelsSpecularIBL);
    
    // -------------------------------------------------------------------------------------
    // Combination of lighting
    // -------------------------------------------------------------------------------------
    vec3 Luminance = DiffuseIBL.rgb + SpecularIBL.rgb;
    
//    AverageExposure = 0.0f;

    // -------------------------------------------------------------------------------------
    // Output
    // -------------------------------------------------------------------------------------
    float Alpha = textureLod(ps_ShadowCubemap, WSReflectVector, 0).r < 1.0f ? 1.0f : 0.0f;

    out_Output = vec4(Luminance * AverageExposure, Alpha);
}

#endif // __INCLUDE_FS_LIGHT_IMAGELIGHT_GLSL__