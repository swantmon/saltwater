
#ifndef __INCLUDE_FS_LIGHTPROBE_SPECULAR_SAMPLING_GLSL__
#define __INCLUDE_FS_LIGHTPROBE_SPECULAR_SAMPLING_GLSL__

#include "common_light.glsl"


// -----------------------------------------------------------------------------
// Defines
// -----------------------------------------------------------------------------
#define NUM_SAMPLES 32

// -----------------------------------------------------------------------------
// Input from engine
// -----------------------------------------------------------------------------
layout(row_major, std140, binding = 0) uniform UB0
{
    vec4 ps_ConstantBufferData0;
};

uniform samplerCube ps_EnvironmentCubemap;

// -----------------------------------------------------------------------------
// Easy access defines
// -----------------------------------------------------------------------------
#define ps_LinearRoughness   ps_ConstantBufferData0.x
#define ps_NumberOfMiplevels ps_ConstantBufferData0.y

// -----------------------------------------------------------------------------
// Input to fragment from VS
// -----------------------------------------------------------------------------
layout(location = 0) in vec3 in_Normal;

// -----------------------------------------------------------------------------
// Output to fragment
// -----------------------------------------------------------------------------
layout(location = 0) out vec4 out_Output;

// -----------------------------------------------------------------------------
// Functions
// -----------------------------------------------------------------------------
vec3 GetImportanceSampleSpecular(in vec3 _Reflection, in float _Roughness)
{
    vec3  Result = vec3(0.0f);
    float Weight = 0.0f;
    
    vec3 N = _Reflection;
    vec3 V = _Reflection;

    const uint NumSamples = NUM_SAMPLES;
    
    ivec2 CubeSize = textureSize(ps_EnvironmentCubemap, 0);
    
    for(uint IndexOfSample = 0; IndexOfSample < NumSamples; IndexOfSample++ )
    {
        vec2  Xi    = GetHammersley(IndexOfSample, NumSamples);
        vec3  H     = GetImportanceSampleGGX( Xi, _Roughness, N);
        vec3  L     = 2.0f * dot( V, H ) * H - V;
        float NdotL = max(dot(N, L), 0.0f);
        float NdotH = clamp(dot(N, H), 0.0f, 1.0f);
        
        if (NdotL > 0.0f)
        {
            // -----------------------------------------------------------------------------
            // Use pre -filtered importance sampling (i.e use lower mipmap
            // level for fetching sample with low probability in order
            // to reduce the variance).
            // (Reference: GPU Gem3)
            //
            // Since we pre -integrate the result for normal direction ,
            // N == V and then NdotH == LdotH. This is why the BRDF pdf
            // can be simplifed from:
            // pdf = D_GGX_Divide_Pi(NdotH , roughness)*NdotH /(4* LdotH);
            // to
            // pdf = D_GGX_Divide_Pi(NdotH , roughness) / 4;
            //
            // The mipmap level is clamped to something lower than 8x8
            // in order to avoid cubemap filtering issues
            //
            // - OmegaS: Solid angle associated to a sample
            // - OmegaP: Solid angle associated to a pixel of the cubemap
            // -----------------------------------------------------------------------------
            float PDF = GetDistributionGGX(NdotH, _Roughness) * INV_PI / 4.0f;
            
            float OmegaS   = 1.0 / (NumSamples * PDF);
            float OmegaP   = 4.0 * PI / (6.0f * CubeSize.x * CubeSize.x);
            
            const float LODBias = 1.0f;
            
            float LOD   = _Roughness == 0.0f ? 0.0f : clamp(0.5f * log2(OmegaS / OmegaP) + LODBias, 0.0f, ps_NumberOfMiplevels);
            vec4  Pixel = textureLod(ps_EnvironmentCubemap, L, LOD);

            Result += Pixel.rgb * NdotL;
            Weight += NdotL;
        }
    }

    return Result * (1.0f / Weight);
}

// -----------------------------------------------------------------------------
// Main
// -----------------------------------------------------------------------------
void main(void)
{
    vec3 Reflection = normalize(in_Normal);
    
    vec3 SpecularSample = GetImportanceSampleSpecular(Reflection, ps_LinearRoughness);
    
    out_Output = vec4(SpecularSample, 1.0f);
}

#endif // __INCLUDE_FS_LIGHTPROBE_SPECULAR_SAMPLING_GLSL__