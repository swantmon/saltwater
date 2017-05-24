
#ifndef __INCLUDE_FS_LIGHT_IMAGELIGHT_GLSL__
#define __INCLUDE_FS_LIGHT_IMAGELIGHT_GLSL__

#include "common.glsl"
#include "common_light.glsl"
#include "common_gbuffer.glsl"
#include "common_global.glsl"

// -----------------------------------------------------------------------------
// Definitions
// -----------------------------------------------------------------------------
#define USE_SSR
#define USE_IBL
#define USE_PARALLAX
#define MAX_NUMBER_OF_PROBES 4

#define SKY_PROBE 1
#define LOCAL_PROBE 2

struct SProbeProperties
{
    vec4 ps_ProbePosition;
    vec4 ps_LightSettings;
    uint ps_ProbeType;
};

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

layout(std430, row_major, binding = 1) readonly buffer BB1
{
    SProbeProperties ps_LightProperties[MAX_NUMBER_OF_PROBES];
};

layout(binding =  0) uniform sampler2D   ps_GBuffer0;
layout(binding =  1) uniform sampler2D   ps_GBuffer1;
layout(binding =  2) uniform sampler2D   ps_GBuffer2;
layout(binding =  3) uniform sampler2D   ps_Depth;
layout(binding =  4) uniform sampler2D   ps_BRDF;
layout(binding =  5) uniform sampler2D   ps_SSR;
layout(binding =  6) uniform samplerCube ps_SpecularCubemap[MAX_NUMBER_OF_PROBES];
layout(binding = 10) uniform samplerCube ps_DiffuseCubemap[MAX_NUMBER_OF_PROBES];
layout(binding = 14) uniform samplerCube ps_ShadowCubemap[MAX_NUMBER_OF_PROBES];

// -----------------------------------------------------------------------------
// Easy access
// -----------------------------------------------------------------------------
#define ps_ExposureHistoryIndex ps_ConstantBufferData0.x

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
vec3 GetParallaxReflection(in vec3 _WSReflectVector, in vec3 _CubemapWSPosition, in vec3 _WSPosition, in vec2 _BoxMinMax)
{
    // -----------------------------------------------------------------------------
    // Special thanks to seblagarde
    // More information here: https://seblagarde.wordpress.com/2012/09/29/image-based-lighting-approaches-and-parallax-corrected-cubemap/
    // -----------------------------------------------------------------------------
    vec3 FirstPlaneIntersect  = (_BoxMinMax.y - _WSPosition) / _WSReflectVector;
    vec3 SecondPlaneIntersect = (_BoxMinMax.x - _WSPosition) / _WSReflectVector;

    vec3 FurthestPlane = max(FirstPlaneIntersect, SecondPlaneIntersect);

    float Distance = min(FurthestPlane.x, min(FurthestPlane.y, FurthestPlane.z));

    vec3 IntersectWSPosition = _WSPosition + _WSReflectVector * Distance;

    return IntersectWSPosition - _CubemapWSPosition;
}

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

    // -----------------------------------------------------------------------------
    // Lighting
    // -----------------------------------------------------------------------------
    vec4 SpecularLighting = vec4(0.0f, 0.0f, 0.0f, 1.0f);
    
#ifdef USE_SSR
    vec4 SSR = textureLod(ps_SSR, in_UV, 0);

    SpecularLighting.rgb = SSR.rgb;
    SpecularLighting.a   = 1.0f - SSR.a;
#endif
    
#ifdef USE_IBL
    if (SpecularLighting.a > 0.001f)
    { 
        vec4 IBL = vec4(0.0f);
        
        #pragma unroll
        for (uint IndexOfLight = 0; IndexOfLight < MAX_NUMBER_OF_PROBES; ++ IndexOfLight)
        {
            SProbeProperties LightProb = ps_LightProperties[IndexOfLight];

            float NumberOfMiplevelsSpecularIBL = LightProb.ps_LightSettings.x;

            if (LightProb.ps_ProbeType != 0)
            {
#ifdef USE_PARALLAX
                // -------------------------------------------------------------------------------------
                // Cumpute reflection vector based on parallax correction
                // -------------------------------------------------------------------------------------
                WSReflectVector = normalize(reflect(WSViewDirection, Data.m_WSNormal));

                if (LightProb.ps_LightSettings.y == 1.0f)
                {
                    vec2 BoxMinMax = vec2(LightProb.ps_LightSettings.z, LightProb.ps_LightSettings.w);

                    WSReflectVector = GetParallaxReflection(WSReflectVector, LightProb.ps_ProbePosition.xyz, Data.m_WSPosition, BoxMinMax);
                }
#endif

                // -------------------------------------------------------------------------------------
                // Estimate lighting of cubemap
                // -------------------------------------------------------------------------------------
                vec3 DiffuseIBL  = EvaluateDiffuseIBL(ps_DiffuseCubemap[IndexOfLight], Data, WSViewDirection, PreDFGF.z, NdotV);
                vec3 SpecularIBL = EvaluateSpecularIBL(ps_SpecularCubemap[IndexOfLight], Data, WSReflectVector, PreDFGF.xy, ClampNdotV, NumberOfMiplevelsSpecularIBL);
                
                // -------------------------------------------------------------------------------------
                // Combination of lighting based on luminosity
                // -------------------------------------------------------------------------------------
                IBL.rgb += (DiffuseIBL.rgb + SpecularIBL.rgb) * AverageExposure * (1.0f - IBL.a);
                
                float Luminosity = 0.3f * IBL.r + 0.59f * IBL.g + 0.11f * IBL.b;
                
                IBL.a = min(IBL.a + Luminosity, 1.0f);
            }
        }
        
        SpecularLighting += SpecularLighting.a * vec4(IBL.rgb, 0.0f);
    }
#endif
    
    // -------------------------------------------------------------------------------------
    // Output
    // -------------------------------------------------------------------------------------    
    out_Output = vec4(SpecularLighting.rgb, 0.0f);
}

#endif // __INCLUDE_FS_LIGHT_IMAGELIGHT_GLSL__