
#ifndef __INCLUDE_FS_MATERIAL_FORWARD_GLSL__
#define __INCLUDE_FS_MATERIAL_FORWARD_GLSL__

#include "common.glsl"
#include "common_light.glsl"
#include "common_global.glsl"
#include "common_gbuffer.glsl"
#include "common_material.glsl"

// -----------------------------------------------------------------------------
// Definitions
// -----------------------------------------------------------------------------
#ifndef MAX_NUMBER_OF_LIGHTS
#define MAX_NUMBER_OF_LIGHTS 4
#endif

#define SUN_LIGHT 1u
#define POINT_LIGHT 2u
#define LIGHT_PROBE 3u

struct SLightProperties
{
    mat4 ps_LightViewProjection;
    vec4 ps_LightPosition;
    vec4 ps_LightDirection;
    vec4 ps_LightColor;
    vec4 ps_LightSettings;
    uint ps_LightType;
};

// -----------------------------------------------------------------------------
// Input from engine
// -----------------------------------------------------------------------------
layout(std140, binding = 3) uniform UB3
{
    vec4  ps_TilingOffset;
    vec4  ps_Color;
    float ps_Roughness;
    float ps_Reflectance;
    float ps_MetalMask;
};

layout(std430, binding = 0) readonly buffer BB1
{
    SLightProperties ps_LightProperties[MAX_NUMBER_OF_LIGHTS];
};

layout (binding = 0) uniform sampler2D ps_BackgroundColor;
layout (binding = 1) uniform sampler2DShadow ps_ShadowTexture[MAX_NUMBER_OF_LIGHTS];

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
    float Alpha     = ps_Color.w;
    vec3 Luminance  = vec3(0.0f);

    vec2 ScreenUV = vec2(gl_FragCoord.xy * g_InvertedScreensizeAndScreensize.xy);
    
    Color *= texture(ps_BackgroundColor, ScreenUV).rgb;

    // -----------------------------------------------------------------------------
    // Surface data
    // -----------------------------------------------------------------------------
    SGBuffer GBuffer;

    PackGBuffer(Color, WSNormal, Roughness, vec3(ps_Reflectance), MetalMask, AO, GBuffer);

    SSurfaceData Data;

    UnpackGBuffer(GBuffer.m_Color0, GBuffer.m_Color1, GBuffer.m_Color2, in_Position.xyz, gl_FragCoord.z, Data);

    // -----------------------------------------------------------------------------
    // Forward pass for each light
    // -----------------------------------------------------------------------------
    #pragma unroll
    for (int IndexOfLight = 0; IndexOfLight < MAX_NUMBER_OF_LIGHTS; ++ IndexOfLight)
    {
        SLightProperties LightProb = ps_LightProperties[IndexOfLight];

        if (LightProb.ps_LightType == SUN_LIGHT)
        {            
            // -----------------------------------------------------------------------------
            // Compute attenuation
            // -----------------------------------------------------------------------------
            float Attenuation = 1.0f;
            Attenuation *= Data.m_AmbientOcclusion;
            Attenuation *= GetShadowAtPositionWithPCF(Data.m_WSPosition, LightProb.ps_LightViewProjection, ps_ShadowTexture[IndexOfLight]);

            // -----------------------------------------------------------------------------
            // Apply light luminance
            // -----------------------------------------------------------------------------
            Luminance += Color * Attenuation;
        }
    }

    out_Output = vec4(Luminance, Alpha);
}

#endif // __INCLUDE_FS_MATERIAL_FORWARD_GLSL__