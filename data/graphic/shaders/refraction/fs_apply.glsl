
#ifndef __INCLUDE_FS_APPLY_GLSL__
#define __INCLUDE_FS_APPLY_GLSL__

#include "common.glsl"
#include "common_light.glsl"
#include "common_global.glsl"
#include "common_gbuffer.glsl"
#include "common_material.glsl"

// -----------------------------------------------------------------------------
// Definitions
// -----------------------------------------------------------------------------
#define MAX_NUMBER_OF_LIGHTS 4

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

layout(std140, binding = 4) uniform UB4
{
    vec4 ps_CameraPosition;
    uint ps_ExposureHistoryIndex;
};

layout(std140, binding = 5) uniform UB5
{
    vec4 ps_RefractionIndices;
    vec4 ps_DepthLinearization;
};

layout(std430, binding = 0) readonly buffer BB0
{
    float ps_ExposureHistory[8];
};

layout(std430, binding = 1) readonly buffer BB1
{
    SLightProperties ps_LightProperties[MAX_NUMBER_OF_LIGHTS];
};

layout(binding =  0) uniform sampler2D ps_DiffuseTexture;
layout(binding =  1) uniform sampler2D ps_NormalTexture;
layout(binding =  2) uniform sampler2D ps_RougnessTexture;
layout(binding =  3) uniform sampler2D ps_Metaltexture;
layout(binding =  4) uniform sampler2D ps_AOTexture;
// binding 5 is reserved for bump texture
layout(binding =  6) uniform sampler2D ps_AlphaTexture;

layout(binding =  7) uniform sampler2D ps_RefractiveNormal;
layout(binding =  8) uniform sampler2D ps_RefractiveDepth;

layout(binding =  9) uniform sampler2D   ps_BRDF;
layout(binding = 10) uniform samplerCube ps_SpecularCubemap;
layout(binding = 11) uniform samplerCube ps_DiffuseCubemap;
layout(binding = 12) uniform sampler2DShadow ps_ShadowTexture[MAX_NUMBER_OF_LIGHTS];

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
vec2 GetUVFromVSPosition(in vec4 _VSPosition)
{
    vec4 SSPosition = g_ViewToScreen * _VSPosition;

    return 0.5f * (SSPosition.xy / SSPosition.w) + 0.5f;
}

// -----------------------------------------------------------------------------

vec4 GetRefraction(in vec3 _IncidentRay, in vec3 _VSNormal, in float _RefractionIndex, in float _RefractionIndexSqr )
{
    float IdotN = dot(-_IncidentRay, _VSNormal);

    float CosineSqr = 1.0f - _RefractionIndexSqr * (1.0f - IdotN * IdotN);

    return CosineSqr <= 0.0f ? 
        vec4( reflect( _IncidentRay, _VSNormal ).xyz, -1.0f ) : 
        vec4( normalize( _RefractionIndex * _IncidentRay + (_RefractionIndex * IdotN - sqrt( CosineSqr )) * _VSNormal ).xyz, 1.0f); 
}

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
    // Exposure data
    // -----------------------------------------------------------------------------
    float AverageExposure = ps_ExposureHistory[ps_ExposureHistoryIndex];
 
    // -----------------------------------------------------------------------------
    // Stuff that we know from the beginning
    // -----------------------------------------------------------------------------
    vec3 VSNormal   = (g_WorldToView * vec4(WSNormal, 0.0f)).xyz;
    vec3 VSPosition = (g_WorldToView * vec4(Data.m_WSPosition, 1.0f)).xyz;

    vec3 VSNormalSurface1 = normalize(VSNormal);
    vec3 ViewDirection    = normalize(VSPosition);

    // -----------------------------------------------------------------------------
    // Find the distance to front & back surface, first as normalized [0..1] 
    // values, than unprojected
    // -----------------------------------------------------------------------------
    vec2 DepthOfBackAndObject = vec2(texture(ps_RefractiveDepth, UV).x, Data.m_VSDepth);

    DepthOfBackAndObject = ps_DepthLinearization.x / (DepthOfBackAndObject * ps_DepthLinearization.y - ps_DepthLinearization.z );

    float Distance = DepthOfBackAndObject.y - DepthOfBackAndObject.x;
    
    // -----------------------------------------------------------------------------
    // Find the refraction direction of first surface
    // -----------------------------------------------------------------------------
    vec3 RefractionSurface1 = GetRefraction(ViewDirection, VSNormalSurface1, ps_RefractionIndices.x, ps_RefractionIndices.y).xyz; 

    // -----------------------------------------------------------------------------
    // Compute approximate exitant location & surface normal
    // -----------------------------------------------------------------------------
    vec4 VSExitantLocation = vec4(RefractionSurface1 * Distance + VSPosition.xyz, 1.0f);

    vec3 WSNormalSurf2 = texture(ps_RefractiveNormal, GetUVFromVSPosition(VSExitantLocation)).xyz;

    vec3 VSNormalSurface2 = (g_WorldToView * vec4(WSNormalSurf2, 0.0f)).xyz;

    float NdotN = dot(VSNormalSurface2.xyz, VSNormalSurface2.xyz);

    VSNormalSurface2 = normalize( VSNormalSurface2 );

    // -----------------------------------------------------------------------------
    // What happens if we lie in a black-texel? Means no normal! Conceptually,
    // this means we pass thru "side" of object. Use norm perpindicular to view
    // -----------------------------------------------------------------------------
    if (NdotN == 0.0f) VSNormalSurface2 = normalize(vec3(RefractionSurface1.x, RefractionSurface1.y, 0.0f));

    // -----------------------------------------------------------------------------
    // Refract at the second surface
    // -----------------------------------------------------------------------------
    vec4 RefractionSurface2 = GetRefraction(RefractionSurface1, -VSNormalSurface2, ps_RefractionIndices.z, ps_RefractionIndices.w );

    vec4 WSRefraction = g_ViewToWorld * vec4(RefractionSurface2.xyz, 0.0f);

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
            // Compute lighting for sun light
            // -----------------------------------------------------------------------------
            vec3 WSLightDirection  = -LightProb.ps_LightDirection.xyz;
            vec3 WSViewDirection   = normalize(ps_CameraPosition.xyz - Data.m_WSPosition);
            
            float NdotV = dot(Data.m_WSNormal, WSViewDirection);
            
            vec3 ViewMirrorUnitDir = 2.0f * NdotV * Data.m_WSNormal - WSViewDirection;

            // -----------------------------------------------------------------------------
            // Compute sun data
            // -----------------------------------------------------------------------------
            float r = sin(LightProb.ps_LightSettings[0]);
            float d = cos(LightProb.ps_LightSettings[0]);

            float DdotR = dot(WSLightDirection, ViewMirrorUnitDir);
            vec3  S     = ViewMirrorUnitDir - DdotR * WSLightDirection;
            vec3  L     = DdotR < d ? normalize(d * WSLightDirection + normalize(S) * r) : ViewMirrorUnitDir;
            
            // -----------------------------------------------------------------------------
            // Compute attenuation
            // -----------------------------------------------------------------------------
            float Attenuation = 1.0f;
            Attenuation *= Data.m_AmbientOcclusion;
            Attenuation *= GetShadowAtPositionWithPCF(Data.m_WSPosition, LightProb.ps_LightViewProjection, ps_ShadowTexture[IndexOfLight]);

            // -----------------------------------------------------------------------------
            // Apply light luminance
            // -----------------------------------------------------------------------------
            Luminance += BRDF(L, WSViewDirection, Data.m_WSNormal, Data) * clamp(dot(Data.m_WSNormal, L), 0.0f, 1.0f) * LightProb.ps_LightColor.xyz * Attenuation * AverageExposure;
        }
        else if (LightProb.ps_LightType == POINT_LIGHT)
        {
            // -----------------------------------------------------------------------------
            // Light data
            // -----------------------------------------------------------------------------
            float LightInvSqrAttenuationRadius = LightProb.ps_LightSettings.x;
            float LightAngleScale              = LightProb.ps_LightSettings.y;
            float LightAngleOffset             = LightProb.ps_LightSettings.z;
            float LightHasShadows              = LightProb.ps_LightSettings.w;

            // -----------------------------------------------------------------------------
            // Compute lighting for punctual lights
            // -----------------------------------------------------------------------------
            vec3 UnnormalizedLightVector = LightProb.ps_LightPosition.xyz - Data.m_WSPosition;
            vec3 NormalizedLightVector   = normalize(UnnormalizedLightVector);
            vec3 WSViewDirection         = normalize(ps_CameraPosition.xyz - Data.m_WSPosition);

            // -----------------------------------------------------------------------------
            // Compute attenuation
            // -----------------------------------------------------------------------------
            float Attenuation = 1.0f;

            Attenuation *= GetDistanceAttenuation(UnnormalizedLightVector, LightInvSqrAttenuationRadius);
            Attenuation *= GetAngleAttenuation(NormalizedLightVector, -LightProb.ps_LightDirection.xyz, LightAngleScale, LightAngleOffset);

            // -----------------------------------------------------------------------------
            // Shadowing
            // -----------------------------------------------------------------------------
            Attenuation *= Data.m_AmbientOcclusion;
            Attenuation *= LightHasShadows == 1.0f ? GetShadowAtPositionWithPCF(Data.m_WSPosition, LightProb.ps_LightViewProjection, ps_ShadowTexture[IndexOfLight]) : 1.0f;
            
            // -----------------------------------------------------------------------------
            // Apply light luminance and shading
            // -----------------------------------------------------------------------------
            Luminance += BRDF(NormalizedLightVector, WSViewDirection, Data.m_WSNormal, Data) * clamp(dot(Data.m_WSNormal, NormalizedLightVector), 0.0f, 1.0f) * LightProb.ps_LightColor.xyz * Attenuation * AverageExposure;
        }
        else if (LightProb.ps_LightType == LIGHT_PROBE)
        {
            // -----------------------------------------------------------------------------
            // Light data
            // -----------------------------------------------------------------------------
            float NumberOfMiplevels = LightProb.ps_LightSettings.x;

            // -----------------------------------------------------------------------------
            // Compute lighting for sphere lights
            // -----------------------------------------------------------------------------
            vec3  WSViewDirection = normalize(Data.m_WSPosition - ps_CameraPosition.xyz);
            float NdotV           = clamp( dot( Data.m_WSNormal, -WSViewDirection ), 0.0, 1.0f);

            // -----------------------------------------------------------------------------
            // Rebuild the function
            // -----------------------------------------------------------------------------
            ivec2 DFGSize = textureSize(ps_BRDF, 0);

            float ClampNdotV = max(NdotV, 0.5f / float(DFGSize.x));
            
            // -----------------------------------------------------------------------------
            // Get data
            // -----------------------------------------------------------------------------
            vec3 PreDFGF = textureLod(ps_BRDF, vec2(NdotV, Data.m_Roughness), 0.0f).rgb;
            
            vec3 DiffuseIBL  = EvaluateDiffuseIBL(ps_DiffuseCubemap, Data, WSViewDirection, PreDFGF.z, NdotV);
            vec3 SpecularIBL = EvaluateSpecularIBL(ps_SpecularCubemap, Data, normalize(WSRefraction.xyz), PreDFGF.xy, ClampNdotV, NumberOfMiplevels);
            
            // -------------------------------------------------------------------------------------
            // Combination of lighting
            // Multiplication with average exposure is not necessary because it is already included
            // -------------------------------------------------------------------------------------
            Luminance += (DiffuseIBL.rgb + SpecularIBL.rgb);
        }
    }

    out_Output = vec4(Luminance, Alpha);
}

#endif // __INCLUDE_FS_APPLY_GLSL__