
#ifndef __INCLUDE_COMMON_LIGHT_GLSL__
#define __INCLUDE_COMMON_LIGHT_GLSL__

#include "common.glsl"
#include "common_material.glsl"

// -----------------------------------------------------------------------------
// Settings
// -----------------------------------------------------------------------------

// Diffuse model:
// 0: Lambert
// 1: Disney Original
// 2: Disney renormalized
#define BRDF_DIFFUSE 2

// Microfacet distribution function:
// 0: GGX
// 1: Beckmann
#define BRDF_SPEC_D 0

// Geometric attenuation or shadowing:
// 0: Smith (matched to GGX)
// 1: Smith Correlated (matched to GGX)
// 2: Schlick
#define BRDF_SPEC_G 1

// Fresnel:
// 0: None
// 1: Schlick
// 2: Fresnel
#define BRDF_SPEC_F 1

// -----------------------------------------------------------------------------
// Shadowing
// -----------------------------------------------------------------------------
float GetShadowAtPosition(in vec3 _WSPosition, in mat4 _LightViewProjection, in sampler2D _Shadowmap)
{
    vec4  LSPosition;
    vec3  ShadowCoord;
    float DepthValue;
    float Shadow;

    // -----------------------------------------------------------------------------
    // Set worls space coord into light projection by multiply with light
    // view and projection matrix;
    // -----------------------------------------------------------------------------
    LSPosition = _LightViewProjection * vec4(_WSPosition, 1.0f);
    
    // -----------------------------------------------------------------------------
    // Divide xyz by w to get the position in light view's clip space.
    // -----------------------------------------------------------------------------
    LSPosition.xyz /= LSPosition.w;
    
    // -----------------------------------------------------------------------------
    // Get uv texcoords for this position
    // -----------------------------------------------------------------------------
    ShadowCoord = LSPosition.xyz * 0.5f + 0.5f;
    
    // -----------------------------------------------------------------------------
    // Get final depth at this texcoord and compare it with the real
    // position z value (do a manual depth test)
    // -----------------------------------------------------------------------------
    DepthValue = texture( _Shadowmap, vec2(ShadowCoord.x, ShadowCoord.y) ).r;
    
    Shadow = 1.0f;
    
    if (ShadowCoord.z - 0.001f > DepthValue)
    {
        Shadow = 0.0f;
    }
    
    return Shadow;
}

// -----------------------------------------------------------------------------

float GetShadowAtPositionWithPCF(in vec3 _WSPosition, in mat4 _LightViewProjection, in sampler2D _Shadowmap)
{
    vec4  LSPosition;
    vec3  ShadowCoord;
    vec2  ShadowMapSize;
    float DepthValue;
    float Shadow;
    float ShadowAcc;
    
    // -----------------------------------------------------------------------------
    // Get shadowmap size
    // -----------------------------------------------------------------------------
    ShadowMapSize = textureSize(_Shadowmap, 0);
    
    // -----------------------------------------------------------------------------
    // Set worls space coord into light projection by multiply with light
    // view and projection matrix;
    // -----------------------------------------------------------------------------
    LSPosition = _LightViewProjection * vec4(_WSPosition, 1.0f);
    
    // -----------------------------------------------------------------------------
    // Divide xyz by w to get the position in light view's clip space.
    // -----------------------------------------------------------------------------
    LSPosition.xyz /= LSPosition.w;
    
    // -----------------------------------------------------------------------------
    // Get uv texcoords for this position
    // -----------------------------------------------------------------------------
    ShadowCoord = LSPosition.xyz * 0.5f + 0.5f;
    
    // -----------------------------------------------------------------------------
    // Get final depth at this texcoord and compare it with the real
    // position z value (do a manual depth test)
    // -----------------------------------------------------------------------------
    ShadowAcc = 0.0f;
    Shadow    = 0.0f;
    
    for(int TexCoordX = -1; TexCoordX <= 1; ++TexCoordX)
    {
        for(int TexCoordY = -1; TexCoordY <= 1; ++TexCoordY)
        {
            vec2 Offset = ShadowCoord.xy + (vec2(TexCoordX, TexCoordY) / ShadowMapSize);
            
            DepthValue = texture(_Shadowmap, Offset).r;
            
            Shadow = 1.0f;
            
            if (ShadowCoord.z - 0.001f > DepthValue)
            {
                Shadow = 0.0f;
            }
            
            ShadowAcc += Shadow;
        }
    }
    
    return ShadowAcc / 9.0f;
}

// -----------------------------------------------------------------------------
// Attenuation
// -----------------------------------------------------------------------------
float GetSmoothDistanceAttentuation(in float _SquaredDistance, in float _InverseSqrtAttentuationradius)
{
    float Factor;
    float SmoothFactor;

    Factor       = _SquaredDistance * _InverseSqrtAttentuationradius;
    SmoothFactor = clamp(1.0f - Factor * Factor, 0.0f, 1.0f);

    return SmoothFactor * SmoothFactor;
}

// -----------------------------------------------------------------------------

float GetDistanceAttenuation(in vec3 _UnnormalizedLightVector, in float _InverseSqrtAttentuationradius)
{
    float SquareDistance;
    float Attenuation;

    SquareDistance = dot(_UnnormalizedLightVector , _UnnormalizedLightVector);
    Attenuation    = 1.0f / (max(SquareDistance , 0.01f * 0.01f));

    Attenuation *= GetSmoothDistanceAttentuation(SquareDistance , _InverseSqrtAttentuationradius);

    return Attenuation;
}

// -----------------------------------------------------------------------------

float GetAngleAttenuation(in vec3 _NormalizedLightVector, in vec3 _LightDirection, in float _LightAngleScale, in float _LightAngleOffset)
{
    float LightAngle;
    float Attenuation;

    // -----------------------------------------------------------------------------
    // On the CPU
    // float _LightAngleScale  = 1.0f / max (0.001f, (CosInner - CosOuter));
    // float _LightAngleOffset = -CosOuter * AngleScale;
    // -----------------------------------------------------------------------------
    LightAngle  = dot(_LightDirection , _NormalizedLightVector);
    Attenuation = clamp(LightAngle * _LightAngleScale + _LightAngleOffset, 0.0f, 1.0f);

    // -----------------------------------------------------------------------------
    // Smooth the transition
    // -----------------------------------------------------------------------------
    Attenuation *= Attenuation;

    return Attenuation;
}

// -----------------------------------------------------------------------------
// Illuminance Area Lights
// -----------------------------------------------------------------------------
float GetIlluminanceSphereOrDisk(in float _CosTheta, in float _SinSigmaSqr)
{
    float SinTheta;
    float Illuminance;

    SinTheta = sqrt(1.0f - _CosTheta * _CosTheta);
    
    Illuminance = 0.0f;
    
    if (_CosTheta * _CosTheta > _SinSigmaSqr)
    {
        Illuminance = PI * _SinSigmaSqr * clamp(_CosTheta, 0.0f, 1.0f);
    }
    else
    {
        float X;
        float Y;
        float SinThetaSqrtY;

        X = sqrt(1.0f / _SinSigmaSqr - 1.0f);
        Y = -X * (_CosTheta / SinTheta);
        SinThetaSqrtY = SinTheta * sqrt(1.0f - Y * Y);
        
        Illuminance = (_CosTheta * acos(Y) - X * SinThetaSqrtY) * _SinSigmaSqr + atan(SinThetaSqrtY / X);
    }
    
    return Illuminance;
}

// -----------------------------------------------------------------------------
// Image light
// -----------------------------------------------------------------------------
vec3 GetSpecularDominantDir(in vec3 _Normal, in vec3 _Reflection, in float _Roughness)
{
    // -----------------------------------------------------------------------------
    // We have a better approximation of the off specular peak
    // but due to the other approximations we found this one performs better.
    // This approximation works fine for G smith correlated and uncorrelated
    // -----------------------------------------------------------------------------
    float Smoothness = clamp (1.0f - _Roughness, 0.0f, 1.0f);
    float LerpFactor = Smoothness * (sqrt(Smoothness) + _Roughness);
    
    // -----------------------------------------------------------------------------
    // The result is not normalized as we fetch in a cubemap
    // -----------------------------------------------------------------------------
    return mix(_Normal, _Reflection, LerpFactor);
}

// -----------------------------------------------------------------------------

float GetSpecularOcclusion(in float _NdotV , in float _AO, in float _Roughness)
{
    return clamp(pow(_NdotV + _AO, exp2(-16.0f * _Roughness - 1.0f)) - 1.0f + _AO, 0.0f, 1.0f);
}

// -----------------------------------------------------------------------------

vec3 GetDiffuseDominantDir(in vec3 _Normal, in vec3 _ViewDirection, in float _NdotV , in float _Roughness)
{
    float A =  1.023410f * _Roughness - 1.511740f;
    float B = -0.511705f * _Roughness + 0.755868f;
    
    float LerpFactor = clamp (( _NdotV * A + B) * _Roughness, 0.0f, 1.0f);
    
    // -----------------------------------------------------------------------------
    // The result is not normalized as we fetch in a cubemap
    // -----------------------------------------------------------------------------
    return mix(_Normal, _ViewDirection, LerpFactor);
}

// -----------------------------------------------------------------------------
// Importance Sampling
// -----------------------------------------------------------------------------
vec3 GetImportanceSampleCosDirection(in vec2 _U, in vec3 _Normal)
{
    float Phi;
    float ThetaCos;
    float ThetaSin;
    vec3  Dir;
    vec3  BaseX;
    vec3  BaseY;
    vec3  BaseZ;
    
    // -----------------------------------------------------------------------------
    // ThetaCos = sqrt(1.0f - _U.x);
    // ThetaSin = sqrt(1.0f - cosTheta * cosTheta);
    // ThetaSin = sqrt(1.0f - sqrt(1.0f - _U.x) * sqrt(1.0f - _U.x));
    // ThetaSin = sqrt(1.0f - (1.0f - _U.x));
    // ThetaSin = sqrt(_U.x);
    // -----------------------------------------------------------------------------
    Phi      = 2.0f * PI * _U.y;
    ThetaCos = sqrt(max(0.0f, 1.0f - _U.x));
    ThetaSin = sqrt(_U.x);
    
    Dir.x    = ThetaSin * cos(Phi);
    Dir.y    = ThetaSin * sin(Phi);
    Dir.z    = ThetaCos;
    
    Dir = normalize(Dir);

    CalcWorldBase(_Normal, BaseX, BaseY, BaseZ);
    
    return BaseX * Dir.x + BaseY * Dir.y + BaseZ * Dir.z;
}

// -----------------------------------------------------------------------------

vec3 GetImportanceSampleGGX(in vec2 _Xi, in float _Roughness, in vec3 _Normal)
{
    float Alpha;
    float Alpha2;
    float Phi;
    float ThetaCos;
    float ThetaSin;
    vec3  Dir;
    vec3  BaseX;
    vec3  BaseY;
    vec3  BaseZ;

    // -----------------------------------------------------------------------------
    // Base on GGX example in:
    // http://bit.ly/1GdsMUJ
    // -----------------------------------------------------------------------------
    Alpha  = _Roughness * _Roughness;
    Alpha2 = Alpha * Alpha;

    Phi       = 2.0f * PI * _Xi.x;
    ThetaCos  = sqrt((1.0f - _Xi.y) / (1.0f + (Alpha2 - 1.0f) * _Xi.y));
    ThetaSin  = sqrt(1.0f - ThetaCos * ThetaCos);

    Dir.x = ThetaSin * cos( Phi );
    Dir.y = ThetaSin * sin( Phi );
    Dir.z = ThetaCos;

    Dir = normalize(Dir);
    
    CalcWorldBase(_Normal, BaseX, BaseY, BaseZ);

    return BaseX * Dir.x + BaseY * Dir.y + BaseZ * Dir.z;
}

// -----------------------------------------------------------------------------
// Specular & Diffuse Fresnel BRDF
// -----------------------------------------------------------------------------
vec3 GetFresnelNone(in vec3 _F0)
{
    return _F0;
}

// -----------------------------------------------------------------------------

vec3 GetFresnelSchlick(in vec3 _F0, in float _F90, in float _U)
{
    float Fc;

    // -----------------------------------------------------------------------------
    // [Schlick 1994, "An Inexpensive BRDF Model for Physically-Based Rendering"]
    // Optimized version inside the Unreal Engine.
    // Orginal version:
    // return _F0 + (_F90 - _F0) * pow(1.0f - _U, 5.0f);
    // -----------------------------------------------------------------------------
    Fc = pow(1.0f - _U, 5.0f);
    
    // -----------------------------------------------------------------------------
    // Anything less than 2% is physically impossible and is instead considered 
    // to be shadowing
    // -----------------------------------------------------------------------------
    return clamp(50.0f * _F0.g , 0.0f, 1.0f) * Fc + (1 - Fc) * _F0;
}

// -----------------------------------------------------------------------------

vec3 GetFresnelFresnel(in vec3 _F0, in float _U )
{
    vec3 SpecularColorSqrt;
    vec3 Normal;
    vec3 G;
    vec3 GU1;
    vec3 GU2;

    SpecularColorSqrt = sqrt(clamp(vec3(0.0f, 0.0f, 0.0f), vec3(0.99f, 0.99f, 0.99f), _F0 ));

    Normal = ( 1 + SpecularColorSqrt ) / ( 1 - SpecularColorSqrt );

    G = sqrt( Normal * Normal + _U * _U - 1.0f );

    GU1 = (G - _U) / (G + _U);
    GU2 = ((G + _U) * _U - 1.0f) / ((G - _U) * _U + 1.0f);

    return 0.5f * GU1 * GU1 * ( 1 + GU2 * GU2 );
}

// -----------------------------------------------------------------------------

vec3 GetFresnel(in vec3 _F0, in float _F90, in float _U)
{
#if   BRDF_SPEC_F == 0
    return GetFresnelNone(_F0);
#elif BRDF_SPEC_F == 1
    return GetFresnelSchlick(_F0, _F90, _U);
#elif BRDF_SPEC_F == 2
    return GetFresnelFresnel(_F0, _U);
#endif
}

// -----------------------------------------------------------------------------
// Specular BRDF Normal Distribution Functions.
// -----------------------------------------------------------------------------
float GetDistributionGGX(in float _NdotH, in float _Roughness)
{
    float Alpha;
    float Alpha2;
    float F;

    // -----------------------------------------------------------------------------
    // Microfacet distribution - Note: Divide by PI is apply later
    // [Walter et al. 2007, "Microfacet models for refraction through rough 
    // surfaces"]
    // -----------------------------------------------------------------------------
    Alpha  = _Roughness * _Roughness;
    Alpha2 = Alpha * Alpha;
    F      = ( _NdotH * Alpha2 - _NdotH ) * _NdotH + 1.0f;
    
    return Alpha2 / ( F * F );
}

// -----------------------------------------------------------------------------

float GetDistributionBeckmann(in float _NdotH, in float _Roughness)
{
    float Alpha;
    float Alpha2;
    float NdotH2;

    // -----------------------------------------------------------------------------
    // [Beckmann 1963, "The scattering of electromagnetic waves from rough 
    // surfaces"]
    // -----------------------------------------------------------------------------
    Alpha  = _Roughness * _Roughness;
    Alpha2 = Alpha * Alpha;
    NdotH2 = _NdotH * _NdotH;

    return exp((NdotH2 - 1.0f) / (Alpha2 * NdotH2)) / (Alpha2 * NdotH2 * NdotH2);
}

// -----------------------------------------------------------------------------

float GetDistribution(in float _NdotH, in float _Roughness)
{
#if   BRDF_SPEC_D == 0
    return GetDistributionGGX(_NdotH, _Roughness);
#elif BRDF_SPEC_D == 1
    return GetDistributionBeckmann(_NdotH, _Roughness);
#endif
}

// -----------------------------------------------------------------------------
// Specular BRDF visibility
// -----------------------------------------------------------------------------
float GetVisibilitySmithGGX(in float _NdotV, in float _NdotL, in float _Roughness)
{
    // -----------------------------------------------------------------------------
    // Original visibility function by Heitz
    // [Smith 1967, "Geometrical shadowing of a random rough surface"]
    // -----------------------------------------------------------------------------
    float Alpha;
    float Alpha2;
    float LambdaGGXV;
    float LambdaGGXL;
  
    Alpha  = _Roughness * _Roughness;
    Alpha2 = Alpha * Alpha;

    LambdaGGXV = _NdotV + sqrt(_NdotV * (_NdotV - _NdotV * Alpha2) + Alpha2);
    LambdaGGXL = _NdotL + sqrt(_NdotL * (_NdotL - _NdotL * Alpha2) + Alpha2);

    return 1.0f / (LambdaGGXV * LambdaGGXL);
}

// -----------------------------------------------------------------------------

float GetVisibilitySmithGGXCorrelated(in float _NdotV, in float _NdotL, in float _Roughness)
{
    float Alpha;
    float Alpha2;
    float LambdaGGXV;
    float LambdaGGXL;

    // -----------------------------------------------------------------------------
    // Note that this is a visibility function and not only a geometry function, 
    // i.e. it already represents the G / (4.0f * _NdotL * _NdotV) term of the specular 
    // BRDF.
    // Improved by Frostbite in course notes from SIGGGRAPH 2014
    // -----------------------------------------------------------------------------
    Alpha  = _Roughness * _Roughness;
    Alpha2 = Alpha * Alpha;
    
    LambdaGGXV = _NdotL * sqrt( ( -_NdotV * Alpha2 + _NdotV ) * _NdotV + Alpha2 );
    LambdaGGXL = _NdotV * sqrt( ( -_NdotL * Alpha2 + _NdotL ) * _NdotL + Alpha2 );
    
    return 0.5f / ( LambdaGGXV + LambdaGGXL );
}

// -----------------------------------------------------------------------------

float GetVisibilitySchlick(in float _NdotV, in float _NdotL, in float _Roughness)
{
    float Alpha2;
    float SchlickV;
    float SchlickL;

    // -----------------------------------------------------------------------------
    // [Schlick 1994, "An Inexpensive BRDF Model for Physically-Based Rendering"]
    // -----------------------------------------------------------------------------
    Alpha2   = _Roughness * _Roughness * 0.5f;

    SchlickV = _NdotV * (1.0f - Alpha2) + Alpha2;
    SchlickL = _NdotL * (1.0f - Alpha2) + Alpha2;

    return 0.25f / (SchlickV * SchlickL);
}

// -----------------------------------------------------------------------------

float GetVisibility(in float _NdotV, in float _NdotL, in float _Roughness)
{
#if   BRDF_SPEC_G == 0
    return GetVisibilitySmithGGX(_NdotV, _NdotL, _Roughness);
#elif BRDF_SPEC_G == 1
    return GetVisibilitySmithGGXCorrelated(_NdotV, _NdotL, _Roughness);  
#elif BRDF_SPEC_G == 2
    return GetVisibilitySchlick(_NdotV, _NdotL, _Roughness);    
#endif
}

// -----------------------------------------------------------------------------
// Specular BRDF geometry
// -----------------------------------------------------------------------------
float GetGeometrySmithGGXJointApproximation(in float _NdotV, in float _NdotL, in float _Roughness)
{
    float Alpha;
    float Alpha2;
    float LambdaGGXV;
    float LambdaGGXL;

    // -----------------------------------------------------------------------------
    // Appoximation of joint Smith term for GGX
    // [Heitz 2014, "Understanding the Masking-Shadowing Function in 
    // Microfacet-Based BRDFs"]
    // -----------------------------------------------------------------------------
    Alpha  = _Roughness * _Roughness;
    Alpha2 = Alpha * Alpha;

    LambdaGGXV = _NdotL * ( _NdotV * ( 1.0f - Alpha2 ) + Alpha2 );
    LambdaGGXL = _NdotV * ( _NdotL * ( 1.0f - Alpha2 ) + Alpha2 );

    return 0.5f * (1.0f / ( LambdaGGXV + LambdaGGXL ));
}

// -----------------------------------------------------------------------------

float GetGeometrySmithGGXCorrelated(in float _NdotV, in float _NdotL, in float _Roughness)
{
    float NoV2;
    float NoL2;
    float Alpha;
    float Alpha2;
    float LambdaGGXV;
    float LambdaGGXL;

    Alpha      = _Roughness * _Roughness;
    Alpha2     = Alpha * Alpha;

    NoV2       = _NdotV * _NdotV;
    NoL2       = _NdotL * _NdotL;
    
    LambdaGGXV = (sqrt(1.0f + Alpha2 * (1.0f - NoV2) / NoV2) - 1.0f) * 0.5f;
    LambdaGGXL = (sqrt(1.0f + Alpha2 * (1.0f - NoL2) / NoL2) - 1.0f) * 0.5f;

    return 1.0f / (1.0f + LambdaGGXV + LambdaGGXL);
}

// -----------------------------------------------------------------------------

float GetGeometry(in float _NdotV, in float _NdotL, in float _Roughness)
{
#if   BRDF_SPEC_G == 0
    return GetGeometrySmithGGXJointApproximation(_NdotV, _NdotL, _Roughness);
#elif BRDF_SPEC_G == 1
    return GetGeometrySmithGGXCorrelated(_NdotV, _NdotL, _Roughness);  
#elif BRDF_SPEC_G == 2
    return GetGeometrySmithGGXJointApproximation(_NdotV, _NdotL, _Roughness);    
#endif
}

// -----------------------------------------------------------------------------
// Specular BRDF models.
// -----------------------------------------------------------------------------
vec3 GetSpecularBSDF(in vec3 _Albedo, in float _NdotV, in float _NdotL, in float _NdotH, in float _LdotH, in float _Roughness)
{
    vec3  Fresnel;
    float FD90;
    float Visibility;
    float Distribution;

    FD90 = clamp(50.0f * dot(_Albedo, vec3(0.33f)), 0.0f, 1.0f);
    
    Distribution = GetDistribution(_NdotH, _Roughness);
    Fresnel      = GetFresnel(_Albedo, FD90, _LdotH);
    Visibility   = GetVisibility(_NdotV, _NdotL, _Roughness);

    return Distribution * Fresnel * Visibility / PI;
}

// -----------------------------------------------------------------------------
// Diffuse
// -----------------------------------------------------------------------------
float GetDiffuseLambert()
{
    return 1.0f;
}

// -----------------------------------------------------------------------------

float GetDiffuseDisney(in float _NdotV, in float _NdotL, in float _LdotH, in float _Roughness)
{
    float FD90;
    vec3  F0;
    float ViewScatter;
    float LightScatter;
 
    // -----------------------------------------------------------------------------
    // Original diffuse Disney BRDF model which is not energy conservating.
    // -----------------------------------------------------------------------------   
    FD90         = 0.5f + 2.0f * _LdotH * _LdotH * _Roughness;
    F0           = vec3(1.0f, 1.0f, 1.0f);
    ViewScatter  = GetFresnel(F0, FD90, _NdotV).x;
    LightScatter = GetFresnel(F0, FD90, _NdotL).x;
    
    return ViewScatter * LightScatter;
}

// -----------------------------------------------------------------------------

float GetDiffuseRenormalizedDisney(in float _NdotV, in float _NdotL, in float _LdotH, in float _Roughness)
{
    float  EnergyBias;
    float  EnergyFactor;
    float  FD90;
    vec3   F0;
    float  LightScatter;
    float  ViewScatter;

    // -----------------------------------------------------------------------------
    // Normalized, energy conservating diffuse Disney BRDF model as used by DICE.
    // -----------------------------------------------------------------------------
    EnergyBias   = mix(0.0f, 0.5f        , _Roughness);
    EnergyFactor = mix(1.0f, 1.0f / 1.51f, _Roughness);
    
    FD90 = EnergyBias + 2.0f * _LdotH * _LdotH * _Roughness;
    F0   = vec3(1.0f, 1.0f, 1.0f);
    
    LightScatter = GetFresnel(F0 , FD90 , _NdotL).x;
    ViewScatter  = GetFresnel(F0 , FD90 , _NdotV).x;
    
    return LightScatter * ViewScatter * EnergyFactor;
}

// -----------------------------------------------------------------------------

float GetDiffuse(in float _NdotV, in float _NdotL, in float _LdotH, in float _Roughness)
{
#if   BRDF_DIFFUSE == 0
    return GetDiffuseLambert();
#elif BRDF_DIFFUSE == 1
    return GetDiffuseDisney(_NdotV, _NdotL, _LdotH, _Roughness);
#elif BRDF_DIFFUSE == 2
    return GetDiffuseRenormalizedDisney(_NdotV, _NdotL, _LdotH, _Roughness);
#endif
}

// -----------------------------------------------------------------------------
// Diffuse BRDF models
// -----------------------------------------------------------------------------
vec3 GetDiffuseBRDF(in vec3 _Albedo, in float _NdotV, in float _NdotL, in float _LdotH, in float _Roughness)
{
    return GetDiffuse(_NdotV, _NdotL, _LdotH, _Roughness) / PI * _Albedo;
}

// -----------------------------------------------------------------------------
// Shading
// -----------------------------------------------------------------------------

vec3 BRDF(in vec3 _L, in vec3 _V, in vec3 _N, in SSurfaceData _Data)
{
    // -----------------------------------------------------------------------------
    // Bidirectional reflection distribution function
    // -----------------------------------------------------------------------------
    float  NdotV = abs( dot( _N, _V ) ) + 1E-5F;

    vec3   H     = normalize( _V + _L );
    float  LdotH = clamp(dot( _L, H ) , 0.0f, 1.0f);
    float  NdotH = clamp(dot( _N, H ) , 0.0f, 1.0f);
    float  NdotL = clamp(dot( _N, _L ), 0.0f, 1.0f);

    float Roughness = GetClampedRoughness(_Data.m_Roughness);

    // -----------------------------------------------------------------------------
    // Diffuse BRDF
    // -----------------------------------------------------------------------------
    vec3 DiffuseBSDF = GetDiffuseBRDF(_Data.m_DiffuseAlbedo, NdotV, NdotL, LdotH, Roughness) ;

    // -----------------------------------------------------------------------------
    // Specular BRDF
    // -----------------------------------------------------------------------------
    vec3 SpecularBSDF = GetSpecularBSDF(_Data.m_SpecularAlbedo, NdotV, NdotL, NdotH, LdotH, Roughness);

    return DiffuseBSDF + SpecularBSDF;
}

#endif // __INCLUDE_COMMON_LIGHT_GLSL__