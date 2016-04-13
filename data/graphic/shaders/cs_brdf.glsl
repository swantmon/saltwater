
#ifndef __INCLUDE_CS_BRDF_GLSL__
#define __INCLUDE_CS_BRDF_GLSL__

#include "common.glsl"
#include "common_light.glsl"

// -----------------------------------------------------------------------------
// Settings
// -----------------------------------------------------------------------------

// Pre integration BRDF for environment:
// 0: Unreal
// 1: Frostbite
// 2: Frostbite with normal unit dir with z = 1
#define DFG_PREINTEGRATE_MODE 2

// -------------------------------------------------------------------------------------
// Defines
// -------------------------------------------------------------------------------------
#define BRDF_SIZE 128.0f

// -----------------------------------------------------------------------------
// Output
// -----------------------------------------------------------------------------
layout (binding = 0, rgba32f) writeonly uniform image2D PSTextureBRDF;

// -------------------------------------------------------------------------------------
// Functions
// -------------------------------------------------------------------------------------
vec3 IntegrateDFGUnreal(in float _Roughness, in float _NdotV)
{
    const uint NumSamples = 64;

    uint   Sample;
    float  Geometry;
    float  Visibility;
    float  Fresnel;
    float  NdotV;
    float  NdotL;
    float  NdotH;
    float  VdotH;
    float  FD90;
    float  FdV;
    float  FdL;
    vec3   LightUnitDir;
    vec3   HalfUnitDir;
    vec3   NormalUnitDir;
    vec3   Sum;
    vec3   ViewUnitDir;
    vec2   Hammersley;

    NormalUnitDir = vec3(0.0f, 0.0f, 1.0f);
    ViewUnitDir   = vec3(sqrt(1.0f - _NdotV * _NdotV), 0.0f, _NdotV);
    Sum           = vec3(0.0f);

    for(Sample = 0; Sample < NumSamples; ++ Sample)
    {
        // -----------------------------------------------------------------------------
        // Specular
        // -----------------------------------------------------------------------------
        Hammersley = GetHammersley( Sample, NumSamples );

        HalfUnitDir = GetImportanceSampleGGX( Hammersley, _Roughness,  NormalUnitDir).xyz;
        LightUnitDir = 2.0f * dot(ViewUnitDir, HalfUnitDir) * HalfUnitDir - ViewUnitDir;

        NdotL = clamp(LightUnitDir.z               , 0.0f, 1.0f);
        NdotH = clamp(HalfUnitDir.z                , 0.0f, 1.0f);
        VdotH = clamp(dot(ViewUnitDir, HalfUnitDir), 0.0f, 1.0f);

        if( NdotL > 0 )
        {
            Geometry = GetGeometrySmithGGXJointApproximation( _Roughness, _NdotV, NdotL );

            // -----------------------------------------------------------------------------
            // Incident light = NdotL
            // pdf = D * NdotH / (4 * VdotH)
            // NdotL * Geometry / pdf
            // -----------------------------------------------------------------------------
            Visibility = NdotL * Geometry * (4.0f * VdotH / NdotH);

            Fresnel = pow( 1.0f - VdotH, 5.0f );

            Sum.x += (1.0f - Fresnel) * Visibility;
            Sum.y +=         Fresnel  * Visibility;
        }

        // -----------------------------------------------------------------------------
        // Diffuse
        // -----------------------------------------------------------------------------
        LightUnitDir = GetImportanceSampleCosDirection(Hammersley, NormalUnitDir).xyz;
        HalfUnitDir  = normalize(ViewUnitDir + LightUnitDir);

        FD90 = (0.5f + 2.0f * VdotH * VdotH) * _Roughness;

        FdV  = 1.0f + (FD90 - 1.0f) * pow(1.0f - _NdotV, 5.0f);
        FdL  = 1.0f + (FD90 - 1.0f) * pow(1.0f -  NdotL, 5.0f);

        Sum.z += FdV * FdL * (1.0f - 0.3333f * _Roughness);
    }

    return Sum * (1.0f / float(NumSamples));
}

// -----------------------------------------------------------------------------

vec3 IntegrateDFGFrostbite(in float _Roughness, in float _NdotV)
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
    vec3   Sum;
    vec3   ViewUnitDir;
    vec2   Hammersley;

    ViewUnitDir = vec3(sqrt(1.0f - _NdotV * _NdotV), 0.0f, _NdotV);

    Sum = vec3(0.0f);

    NdotV = clamp(dot(ViewUnitDir, ViewUnitDir), 0.0f, 1.0f);

    for (Sample = 0; Sample < NumSamples; ++ Sample)
    {
        // -----------------------------------------------------------------------------
        // Specular
        // The light vector is the reflection of the view vector and the half vector.
        // -----------------------------------------------------------------------------
        Hammersley   = GetHammersley(Sample, NumSamples);
        
        HalfUnitDir  = GetImportanceSampleGGX(Hammersley, _Roughness, ViewUnitDir);

        LightUnitDir = normalize(2.0f * dot(ViewUnitDir, HalfUnitDir) * HalfUnitDir - ViewUnitDir);

        NdotL        = clamp(dot(ViewUnitDir, LightUnitDir), 0.0f, 1.0f);
        
        Geometry     = GetGeometrySmithGGXCorrelated(NdotV, NdotL, _Roughness);

        if ((NdotL > 0.0f) && (Geometry > 0.0f))
        {
            NdotH        = clamp(dot( ViewUnitDir, HalfUnitDir), 0.0f, 1.0f);
            LdotH        = clamp(dot(LightUnitDir, HalfUnitDir), 0.0f, 1.0f);

            Visibility = Geometry * LdotH / (NdotH * NdotV);

            Fresnel    = pow(1.0f - LdotH, 5.0f);

            Sum.x     += (1.0f - Fresnel) * Visibility;
            Sum.y     +=         Fresnel  * Visibility;
        }

        // -----------------------------------------------------------------------------
        // Diffuse
        // -----------------------------------------------------------------------------
        Hammersley   = fract(Hammersley + 0.5f);

        LightUnitDir = GetImportanceSampleCosDirection(Hammersley, ViewUnitDir);

        NdotL        = clamp(dot(ViewUnitDir, LightUnitDir), 0.0f, 1.0f);

        if (NdotL > 0.0f)
        {
            HalfUnitDir = normalize(ViewUnitDir + LightUnitDir);

            LdotH       = clamp(dot(LightUnitDir, HalfUnitDir), 0.0f, 1.0f);

            Sum.z      += GetDiffuseRenormalizedDisney(NdotV, NdotL, LdotH, _Roughness);
        }
    }

    return Sum * (1.0f / float(NumSamples));
}

// -----------------------------------------------------------------------------

vec3 IntegrateDFGFrostbiteNormalZ(in float _Roughness, in float _NdotV)
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
    float  VdotH;
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
        
        LightUnitDir  = 2.0f * dot(ViewUnitDir, HalfUnitDir) * HalfUnitDir - ViewUnitDir;
        
        NdotL = clamp(LightUnitDir.z, 0.0f, 1.0f);

        NdotH = clamp(HalfUnitDir.z , 0.0f, 1.0f);

        VdotH = clamp(dot(ViewUnitDir, HalfUnitDir), 0.0f, 1.0f);

        Geometry = GetGeometrySmithGGXCorrelated(NdotV, NdotL, _Roughness);
        
        if (NdotL > 0.0f && Geometry > 0.0f)
        {
            Visibility = Geometry * VdotH / (NdotH * NdotV);
            Fresnel    = pow(1.0f - VdotH, 5.0f);
            
            Sum.x += (1.0f - Fresnel) * Visibility;
            Sum.y += Fresnel * Visibility;
        }
        
        // -------------------------------------------------------------------------------------
        // Diffuse
        // -------------------------------------------------------------------------------------
        Hammersley   = fract(Hammersley + 0.5f);
        
        LightUnitDir = GetImportanceSampleCosDirection(Hammersley, NormalUnitDir);

        NdotL        = clamp(dot(LightUnitDir, NormalUnitDir), 0.0f, 1.0f);
        
        if (NdotL > 0.0f)
        {
            LdotH = clamp(dot(LightUnitDir, normalize(ViewUnitDir + LightUnitDir)), 0.0f, 1.0f);

            NdotV = clamp(dot(NormalUnitDir, ViewUnitDir), 0.0f, 1.0f);
            
            Sum.z += GetDiffuseRenormalizedDisney(NdotV , NdotL , LdotH , _Roughness);
        }
    }
    
    return Sum * (1.0f / float(NumSamples));
}

// -----------------------------------------------------------------------------

vec3 IntegrateDFG(in float _Roughness, in float _NdotV)
{
#if   DFG_PREINTEGRATE_MODE == 0
    return IntegrateDFGUnreal(_Roughness, _NdotV);
#elif DFG_PREINTEGRATE_MODE == 1
    return IntegrateDFGFrostbite2(_Roughness, _NdotV);
#elif DFG_PREINTEGRATE_MODE == 2
    return IntegrateDFGFrostbiteNormalZ(_Roughness, _NdotV);
#endif
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
    
    Roughness = float(Y + 0.5f) / BRDF_SIZE;
    NdotV     = float(X + 0.5f) / BRDF_SIZE;
    
    Sum = IntegrateDFG(Roughness, NdotV);

    imageStore(PSTextureBRDF, ivec2(X, Y), vec4(Sum.x, Sum.y, Sum.z, 1.0f));
}

#endif // __INCLUDE_CS_BRDF_GLSL__