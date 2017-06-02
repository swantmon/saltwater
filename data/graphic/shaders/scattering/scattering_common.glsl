////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////// Constants and function for atmospheric scattering
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef __SCATTERING_COMMON__
#define __SCATTERING_COMMON__

#include "scattering/scattering_precompute_common.glsl"

layout(binding = 0) uniform sampler2D g_TransmittanceTable;
layout(binding = 1) uniform sampler3D g_InscatterTable;
layout(binding = 2) uniform sampler2D g_IrradianceTable;
layout(binding = 3) uniform sampler3D g_DeltaSR;
layout(binding = 4) uniform sampler3D g_DeltaSM;
layout(binding = 5) uniform sampler3D g_DeltaJ;
layout(binding = 6) uniform sampler2D g_DeltaE;

layout(std140, binding = 1) uniform SPSPrecomputeConstants
{
    float g_TransmittanceWidth;
    float g_TransmittanceHeight;
    float g_IrradianceWidth;
    float g_IrradianceHeight;
    float g_InscatterMuS;
    float g_InscatterMu;
    float g_InscatterNu;
    float g_InscatterAltitude;
};

//////////////////////////////////////
// Physical constants
//////////////////////////////////////

const float g_HeightScaleRayleigh = 8.0f;
const vec3 g_BetaRayleigh = vec3(5.8e-3f, 1.35e-2f, 3.31e-2f);
const vec3 g_BetaMie = vec3(4e-3f, 4e-3f, 4e-3f);

const float g_HeightScaleMie = 1.2f;
const vec3 g_BetaMieExtinction = g_BetaMie / 0.9f;
const float g_MieG = 0.8f;

const float g_RadiusGround = 6360.0f;
const float g_RadiusAtmosphere = 6420.0f;

const float g_AverageGroundReflectance = 0.1f;

////////////////////////////////////////////////////////////////////////////
// Transmittance coordinates
////////////////////////////////////////////////////////////////////////////

vec2 GetTransmittanceUV(float _Radius, float _Mu) 
{
    float U, V;

	V = sqrt((_Radius - g_RadiusGround) / (g_RadiusAtmosphere - g_RadiusGround));
	U = atan((_Mu + 0.15f) / (1.0f + 0.15f) * tan(1.5f)) / 1.5f;

    return vec2(U, V);
}

vec2 GetTransmittanceRMu(vec2 _UV)
{
    float Radius = _UV.y;
    float Mu = _UV.x;

    Radius = g_RadiusGround + (Radius * Radius) * (g_RadiusAtmosphere - g_RadiusGround);
    Mu = -0.15 + tan(1.5f * Mu) / tan(1.5f) * (1.0f + 0.15f);

    return vec2(Radius, Mu);
}

vec3 GetTransmittance(float _Radius, float _Mu) 
{
	vec2 UV = GetTransmittanceUV(_Radius, _Mu);

    return textureLod(g_TransmittanceTable, UV, 0).rgb;
}

vec3 GetTransmittanceWithShadow(float _Radius, float _Mu)
{
    return _Mu < -sqrt(1.0f- (g_RadiusGround / _Radius) * (g_RadiusGround / _Radius)) ? vec3(0.0f, 0.0f, 0.0f) : GetTransmittance(_Radius, _Mu);
}

vec3 GetTransmittance(float _Radius, float _Mu, float _D)
{
    vec3 Transmittance;
    float R1 = sqrt(_Radius * _Radius + _D * _D + 2.0f * _Radius * _Mu * _D);
    float Mu1 = (_Radius * _Mu + _D) / R1;

    if (_Mu > 0.0f)
    {
        Transmittance = min(GetTransmittance(_Radius, _Mu) / GetTransmittance(R1, Mu1), 1.0f);
    }
    else
    {
        Transmittance = min(GetTransmittance(R1, -Mu1) / GetTransmittance(_Radius, -_Mu), 1.0f);
    }

    return Transmittance;
}

float GetOpticalDepth(float H, float Radius, float Mu, float D) 
{
	float ParticleDensity = 6.2831;
    float A = sqrt((0.5/H)*Radius);
    vec2 A01 = A * vec2(Mu, Mu + D / Radius);
    vec2 A01Sign = sign(A01);
    vec2 A01Squared = A01*A01;
    float X = A01Sign.y > A01Sign.x ? exp(A01Squared.x) : 0.0;
    vec2 Y = A01Sign / (2.3193*abs(A01) + sqrt(1.52*A01Squared + 4.0)) * vec2(1.0, exp(-D/H*(D/(2.0*Radius)+Mu)));

    return sqrt((ParticleDensity*H)*Radius) * exp((g_RadiusGround-Radius)/H) * (X + dot(Y, vec2(1.0, -1.0)));
}

vec3 GetAnalyticTransmittance(float _R, float _Mu, float _D)
{
    return exp(-g_BetaRayleigh * GetOpticalDepth(g_HeightScaleRayleigh, _R, _Mu, _D) - g_BetaMieExtinction * GetOpticalDepth(g_HeightScaleMie, _R, _Mu, _D));
}

////////////////////////////////////////////////////////////////////////////
// Irradiance coordinates
////////////////////////////////////////////////////////////////////////////

vec2 GetIrradianceUV(float _Radius, float _MuS)
{
    float V = (_Radius - g_RadiusGround) / (g_RadiusAtmosphere - g_RadiusGround);
    float U = (_MuS + 0.2f) / (1.0f + 0.2f);

    return vec2(U, V);
}

vec2 GetIrradianceRMuS(vec2 _UV) 
{
    float Radius, MuS;

    Radius = g_RadiusGround + (_UV.y - 0.5f) / (g_IrradianceHeight - 1.0f) * (g_RadiusAtmosphere - g_RadiusGround);
    MuS = -0.2f + (_UV.x - 0.5f) / (g_IrradianceWidth - 1.0f) * (1.0f + 0.2f);

    return vec2(Radius, MuS);
}

////////////////////////////////////////////////////////////////////////////
// Inscatter coordinates
////////////////////////////////////////////////////////////////////////////

vec4 Texture4DSample(in sampler3D _Texture, in float _Radius, in float _Mu, in float _MuS, in float _Nu)
{
    float H = sqrt(g_RadiusAtmosphere * g_RadiusAtmosphere - g_RadiusGround * g_RadiusGround);
    float Rho = sqrt(_Radius * _Radius - g_RadiusGround * g_RadiusGround);

    float RMu = _Radius * _Mu;
    float Delta = RMu * RMu - _Radius * _Radius + g_RadiusGround * g_RadiusGround;

    vec4 TexOffset = RMu < 0.0f && Delta > 0.0f ? vec4(1.0f, 0.0f, 0.0f, 0.5f - 0.5f / g_InscatterMu) : vec4(-1.0f, H * H, H, 0.5f + 0.5f / g_InscatterMu);

	float MuR = 0.5f / g_InscatterAltitude + Rho / H * (1.0f - 1.0f / g_InscatterAltitude);
    float MuMu = TexOffset.w + (RMu * TexOffset.x + sqrt(Delta + TexOffset.y)) / (Rho + TexOffset.z) * (0.5f - 1.0f / g_InscatterMu);
    float MuMuS = 0.5f / g_InscatterMuS + (atan(max(_MuS, -0.1975f) * tan(1.26f * 1.1f)) / 1.1f + (1.0f - 0.26f)) * 0.5f * (1.0f - 1.0f / g_InscatterMuS);
    float LerpValue = (_Nu + 1.0f) * 0.5f * (g_InscatterNu - 1.0f);
    float MuNu = floor(LerpValue);

    LerpValue = LerpValue - MuNu;

    return textureLod(_Texture, vec3((MuNu + MuMuS) / g_InscatterNu, MuMu, MuR), 0) * (1.0f - LerpValue) +
           textureLod(_Texture, vec3((MuNu + MuMuS + 1.0f) / g_InscatterNu, MuMu, MuR), 0) * LerpValue;
}

float Mod(float _X, float _Y)  // HLSL fmod behaves differently than GLSL 
{
	return _X - _Y * floor(_X / _Y);
}

void GetMuMuSNu(vec2 _UV, float _Radius, vec4 _Dhdh, out float _Mu, out float _MuS, out float _Nu)
{
    float InscatterWidth = g_InscatterMuS * g_InscatterNu;

    float X = _UV.x - 0.5f;
    float Y = _UV.y - 0.5f;

    if (Y < g_InscatterMu * 0.5f)
    {
        float D = 1.0f - Y / (g_InscatterMu * 0.5f - 1.0f);
        D = min(max(_Dhdh.z, D * _Dhdh.w), _Dhdh.w * 0.999f);
        _Mu = (g_RadiusGround * g_RadiusGround - _Radius * _Radius - D * D) / (2.0f * _Radius * D);
        _Mu = min(_Mu, -sqrt(1.0f - (g_RadiusGround / _Radius) * (g_RadiusGround / _Radius)) - 0.001f);
    }
    else
    {
        float D = (Y - g_InscatterMu * 0.5f) / (g_InscatterMu * 0.5f - 1.0f);
        D = min(max(_Dhdh.x, D * _Dhdh.y), _Dhdh.y * 0.999f);
        _Mu = (g_RadiusAtmosphere * g_RadiusAtmosphere - _Radius * _Radius - D * D) / (2.0f * _Radius * D);
    }
    _MuS = Mod(X, g_InscatterMuS) / (g_InscatterMuS - 1.0f);
    _MuS = tan((2.0f * _MuS - 1.0f + 0.26f) * 1.1f) / tan(1.26f * 1.1f);
    _Nu = -1.0f + floor(X / g_InscatterMuS) / (g_InscatterNu - 1.0f) * 2.0f;
}

////////////////////////////////////////////////////////////////////////////
// Calculates the length of a ray
// ray hits ground or atmosphere
////////////////////////////////////////////////////////////////////////////

float Limit(float _Radius, float _Mu) 
{
    float Dout = -_Radius * _Mu + sqrt(_Radius * _Radius * (_Mu * _Mu - 1.0) + (g_RadiusAtmosphere + 1) * (g_RadiusAtmosphere + 1));
    float Delta2 = _Radius * _Radius * (_Mu * _Mu - 1.0) + g_RadiusGround * g_RadiusGround;
    if (Delta2 >= 0.0f) 
	{
        float Din = -_Radius * _Mu - sqrt(Delta2);
        if (Din >= 0.0f) 
		{
            Dout = min(Dout, Din);
        }
    }
    return Dout;
}

vec3 GetMie(vec4 _RayleighMie)
{
	return _RayleighMie.rgb * _RayleighMie.w / max(_RayleighMie.r, 1e-4) * (g_BetaRayleigh.r / g_BetaRayleigh);
}

float PhaseFunctionR(float _Mu)
{
    return (3.0f / (16.0f * g_PI)) * (1.0f + _Mu * _Mu);
}

float PhaseFunctionM(float _Mu)
{
	return 1.5f * 1.0f / (4.0f * g_PI) * (1.0f - g_MieG * g_MieG) * pow(abs(1.0f + (g_MieG * g_MieG) - 2.0f * g_MieG * _Mu), -3.0f / 2.0f) * (1.0f + _Mu * _Mu) / (2.0f + g_MieG * g_MieG);
}

vec3 GetIrradiance(in sampler2D _Texture, in float _Radius, in float _MuS)
{
    vec2 UV = GetIrradianceUV(_Radius, _MuS);
    return textureLod(_Texture, UV, 0).rgb;
}

#endif // __SCATTERING_COMMON__