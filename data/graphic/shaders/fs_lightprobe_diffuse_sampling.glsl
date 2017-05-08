
#ifndef __INCLUDE_FS_LIGHTPROBE_DIFFUSE_SAMPLING_GLSL__
#define __INCLUDE_FS_LIGHTPROBE_DIFFUSE_SAMPLING_GLSL__

#include "common.glsl"
#include "common_light.glsl"

// -----------------------------------------------------------------------------
// Defines
// -----------------------------------------------------------------------------
// Down sample methods:
// 0: Frostbite Original (Look strange; need SPH)
// 1: Mipmaped Sampling from IBL baker
#define METHOD 1

#define NUM_SAMPLES 32

// -----------------------------------------------------------------------------
// Input from engine
// -----------------------------------------------------------------------------
layout(row_major, std140, binding = 1) uniform UB0
{
    vec4 ps_ConstantBufferData0;
};

uniform samplerCube ps_EnvironmentCubemap;

// -----------------------------------------------------------------------------
// Easy access defines
// -----------------------------------------------------------------------------
#define ps_Intensity ps_ConstantBufferData0.z

// -----------------------------------------------------------------------------
// Input to fragment from VS
// -----------------------------------------------------------------------------
layout(location = 1) in vec3 in_Normal;

// -----------------------------------------------------------------------------
// Output to fragment
// -----------------------------------------------------------------------------
layout(location = 0) out vec4 out_Output;

// -----------------------------------------------------------------------------
// Functions
// -----------------------------------------------------------------------------
vec3 GetImportanceSampleDiffuseFrostbite(in vec3 _NormalReflection)
{
    vec3  L;
    vec3  Result;
    vec2  Xi;
    float NdotL;
    float PDF;
    
    const uint NumSamples = NUM_SAMPLES;
    
    Result = vec3(0.0f);
    
    for(uint IndexOfSample = 0; IndexOfSample < NumSamples; ++IndexOfSample )
    {
        Xi = GetHammersley(IndexOfSample, NumSamples);
        
        L = GetImportanceSampleCosDirection(Xi, _NormalReflection);
        
        NdotL = dot(L, _NormalReflection);
        
        if (NdotL > 0.0f)
        {
            Result += textureLod(ps_EnvironmentCubemap, L, 0).rgb;
        }
    }
    
    return Result * (1.0f / NumSamples);
}

// -----------------------------------------------------------------------------

vec3 GetImportanceSampleDiffuseMipmapped(in vec3 _NormalReflection)
{
    vec3 V = _NormalReflection;
    vec4 Result = vec4(0.0f);
    
    const uint NumSamples = NUM_SAMPLES;
    
    ivec2 CubeSize = textureSize(ps_EnvironmentCubemap, 0);
    
    for(uint IndexOfSample = 0; IndexOfSample < NumSamples; ++IndexOfSample )
    {
        vec2  Xi  = GetHammersley(IndexOfSample, NumSamples);
        vec3  H   = GetImportanceSampleCosDirection(Xi, _NormalReflection);
        vec3  L   = normalize(2.0f * dot( V, H ) * H - V);
        float NoL = clamp(dot( _NormalReflection, L ), 0.0f, 1.0f);
        
        // -----------------------------------------------------------------------------
        // Compute Lod using inverse solid angle and pdf.
        // From Chapter 20.4 Mipmap filtered samples in GPU Gems 3.
        // http://http.developer.nvidia.com/GPUGems3/gpugems3_ch20.html
        // -----------------------------------------------------------------------------
        float PDF = max(0.0, dot(_NormalReflection, L) * INV_PI);
        
        float SolidAngleTexel  = 4.0f * PI / (6.0f * CubeSize.x * CubeSize.y);
        float SolidAngleSample = 1.0f / (NumSamples * PDF);
        
        //TODO: add mip count to constantbuffer
        float LOD = 0.5f * log2(SolidAngleSample / SolidAngleTexel);
        
        vec3 Pixel = textureLod(ps_EnvironmentCubemap, H, LOD).rgb * ps_Intensity;
        
        Result.xyz += Pixel;
        Result.w++;
    }
    
    if (Result.w == 0.0f)
    {
        return Result.xyz;
    }
    
    return (Result.xyz / Result.w);
}

// -----------------------------------------------------------------------------

vec3 GetImportanceSampleDiffuse(in vec3 _NormalReflection)
{
#if   METHOD == 0
    return GetImportanceSampleDiffuseFrostbite(_NormalReflection);
#elif METHOD == 1
    return GetImportanceSampleDiffuseMipmapped(_NormalReflection);
#endif
}

// -----------------------------------------------------------------------------
// Main
// -----------------------------------------------------------------------------
void main(void)
{
    vec3 Reflection = normalize(in_Normal);
    
    vec3 DiffuseSample = GetImportanceSampleDiffuse(Reflection);
    
    out_Output = vec4(DiffuseSample, 1.0f);
}

#endif // __INCLUDE_FS_LIGHTPROBE_DIFFUSE_SAMPLING_GLSL__