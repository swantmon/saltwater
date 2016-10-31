
#ifndef __INCLUDE_CS_BRDF_GLSL__
#define __INCLUDE_CS_BRDF_GLSL__

#include "common.glsl"
#include "common_light.glsl"

// -------------------------------------------------------------------------------------
// Defines
// -------------------------------------------------------------------------------------
#define BRDF_SIZE 512.0f

// -----------------------------------------------------------------------------
// Output
// -----------------------------------------------------------------------------
layout (binding = 0, rgba32f) writeonly uniform image2D PSTextureBRDF;

// -------------------------------------------------------------------------------------
// Functions
// -------------------------------------------------------------------------------------
vec3 IntegrateDFG(in float _Roughness, in float _NdotV)
{
    const uint NumSamples = 64;

    uint   Sample;
    float  Geometry;
    float  Visibility;
    float  Fresnel;
    float  NdotV;
    float  NdotL;
    float  NdotH;
    float  LdotH;
    vec3   LightUnitDir;
    vec3   HalfUnitDir;
    vec3   NormalUnitDir;
    vec3   Sum;
    vec3   ViewUnitDir;
    vec2   Hammersley;


    NormalUnitDir = vec3(0.0f, 0.0f, 1.0f);
    ViewUnitDir   = vec3(sqrt(1.0f - _NdotV * _NdotV), 0.0f, _NdotV);

    Sum = vec3(0.0f);

    NdotV = clamp(dot(NormalUnitDir, ViewUnitDir), 0.0f, 1.0f);
    
    for (uint IndexOfSample = 0; IndexOfSample < NumSamples; IndexOfSample++)
    {
        // -----------------------------------------------------------------------------
        // Specular
        // -----------------------------------------------------------------------------
        Hammersley    = GetHammersley(IndexOfSample, NumSamples);

        HalfUnitDir   = GetImportanceSampleGGX(Hammersley, _Roughness, NormalUnitDir);
        
        LightUnitDir  = normalize(2.0f * dot(ViewUnitDir, HalfUnitDir) * HalfUnitDir - ViewUnitDir);
        
        NdotL = clamp(LightUnitDir.z, 0.0f, 1.0f);

        Geometry = GetGeometrySmithGGXCorrelated(NdotV, NdotL, _Roughness);
        
        if (NdotL > 0.0f && Geometry > 0.0f)
        {
            NdotH = clamp(dot(NormalUnitDir, HalfUnitDir), 0.0f, 1.0f);

            LdotH = clamp(dot(LightUnitDir, HalfUnitDir), 0.0f, 1.0f);

            Visibility = Geometry * LdotH / (NdotH * NdotV);
            Fresnel    = pow(1.0f - LdotH, 5.0f);
            
            Sum.x += (1.0f - Fresnel) * Visibility;
            Sum.y += Fresnel * Visibility;
        }
        
        // -------------------------------------------------------------------------------------
        // Diffuse
        // -------------------------------------------------------------------------------------
        Hammersley   = fract(Hammersley + 0.5f);
        
        LightUnitDir = GetImportanceSampleCosDirection(Hammersley, NormalUnitDir);

        NdotL        = clamp(dot(NormalUnitDir, LightUnitDir), 0.0f, 1.0f);
        
        if (NdotL > 0.0f)
        {
            LdotH = clamp(dot(LightUnitDir, normalize(ViewUnitDir + LightUnitDir)), 0.0f, 1.0f);
            
            Sum.z += GetDiffuseRenormalizedDisney(NdotV, NdotL , LdotH, _Roughness);
        }
    }
    
    return Sum * (1.0f / float(NumSamples));
}

// -------------------------------------------------------------------------------------

layout (local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main()
{
    uint  X;
    uint  Y;
    float Roughness;
    float NdotV;
    vec3  Sum;

    X = gl_GlobalInvocationID.x;
    Y = gl_GlobalInvocationID.y;
    
    Roughness = float(Y) / BRDF_SIZE;
    NdotV     = float(X) / BRDF_SIZE;
    
    Sum = IntegrateDFG(Roughness, NdotV);

    imageStore(PSTextureBRDF, ivec2(X, Y), vec4(Sum.x, Sum.y, Sum.z, 1.0f));
}

#endif // __INCLUDE_CS_BRDF_GLSL__