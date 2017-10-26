
#ifndef __INCLUDE_FS_INSCATTER_MULTIPLE_A_GLSL__
#define __INCLUDE_FS_INSCATTER_MULTIPLE_A_GLSL__

#include "scattering/scattering_common.glsl"
#include "scattering/scattering_precompute_common.glsl"

// -----------------------------------------------------------------------------
// Defines
// -----------------------------------------------------------------------------
const float g_DeltaPhi = g_PI / float(g_InscatterIntegralSphereSampleCount);
const float g_DeltaTheta = g_PI / float(g_InscatterIntegralSphereSampleCount);

// -----------------------------------------------------------------------------
// Input from engine
// -----------------------------------------------------------------------------
layout(row_major, std140, binding = 3) uniform PSLayerValues
{
    vec4  g_Dhdh;
    float g_Radius;
};

layout(row_major, std140, binding = 4) uniform PSScatteringOrder
{
    float g_FirstOrder;
};

// -----------------------------------------------------------------------------
// Input to fragment from previous stage
// -----------------------------------------------------------------------------
layout(location = 2) in vec2 in_UV;

// -----------------------------------------------------------------------------
// Output to fragment
// -----------------------------------------------------------------------------
layout(location = 0) out vec4 out_Output;

// -----------------------------------------------------------------------------
// Main
// -----------------------------------------------------------------------------
void Inscatter(float _Radius, float _Mu, float _MuS, float _Nu, out vec3 _rRayleighMie)
{
    _Radius = clamp(_Radius, g_RadiusGround, g_RadiusAtmosphere);
    _Mu = clamp(_Mu, -1.0f, 1.0f);
    _MuS = clamp(_MuS, -1.0f, 1.0f);
    float Variation = sqrt(1.0f - _Mu * _Mu) * sqrt(1.0f - _MuS * _MuS);
    _Nu = clamp(_Nu, _MuS * _Mu - Variation, _MuS * _Mu + Variation);

    float CThetaMin = -sqrt(1.0f - (g_RadiusGround / _Radius) * (g_RadiusGround / _Radius));

    vec3 V = vec3(sqrt(1.0f - _Mu * _Mu), 0.0, _Mu);
    float Sx = V.x == 0.0f ? 0.0f : (_Nu - _MuS * _Mu) / V.x;
    vec3 S = vec3(Sx, sqrt(max(0.0f, 1.0f - Sx * Sx - _MuS * _MuS)), _MuS);

    _rRayleighMie = vec3(0.0f);

    for (int iTheta = 0; iTheta < g_InscatterIntegralSphereSampleCount; ++ iTheta) 
	{
        float Theta = (float(iTheta) + 0.5f) * g_DeltaTheta;
        float CTheta = cos(Theta);

        float GReflectance = 0.0f;
        float DGround = 0.0f;
        vec3 GTransmittance = vec3(0.0f, 0.0f, 0.0f);

        if (CTheta < CThetaMin)  // ground visible
		{ 
            GReflectance = g_AverageGroundReflectance / g_PI;
            DGround = -_Radius * CTheta - sqrt(_Radius * _Radius * (CTheta * CTheta - 1.0f) + g_RadiusGround * g_RadiusGround);
            GTransmittance = GetTransmittance(g_RadiusGround, -(_Radius * CTheta + DGround) / g_RadiusGround, DGround);
        }

        for (int iPhi = 0; iPhi < g_InscatterIntegralSphereSampleCount * 2; ++ iPhi) 
		{
            float Phi = (float(iPhi) + 0.5f) * g_DeltaPhi;
            float Dw = g_DeltaTheta * g_DeltaPhi * sin(Theta);
            vec3 W = vec3(cos(Phi) * sin(Theta), sin(Phi) * sin(Theta), CTheta);

            float Nu1 = dot(S, W);
            float Nu2 = dot(V, W);
            float Pr2 = PhaseFunctionR(Nu2);
            float Pm2 = PhaseFunctionM(Nu2);

            vec3 GNormal = (vec3(0.0f, 0.0f, _Radius) + DGround * W) / g_RadiusGround;
            vec3 GIrradiance = GetIrradiance(g_DeltaE, g_RadiusGround, dot(GNormal, S));

            vec3 RayleighMie1 = GReflectance * GIrradiance * GTransmittance;

            if (g_FirstOrder == 1.0f) 
			{
                float Pr1 = PhaseFunctionR(Nu1);
                float Pm1 = PhaseFunctionM(Nu1);
                vec3 RayLeigh1 = Texture4DSample(g_DeltaSR, _Radius, W.z, _MuS, Nu1).rgb;
                vec3 Mie1 = Texture4DSample(g_DeltaSM, _Radius, W.z, _MuS, Nu1).rgb;

                RayleighMie1 += RayLeigh1 * Pr1 + Mie1 * Pm1;
            } 
			else 
			{
                RayleighMie1 += Texture4DSample(g_DeltaSR, _Radius, W.z, _MuS, Nu1).rgb;
            }

            _rRayleighMie += RayleighMie1 * (g_BetaRayleigh * exp(-(_Radius - g_RadiusGround) / g_HeightScaleRayleigh) * Pr2 + g_BetaMie * exp(-(_Radius - g_RadiusGround) / g_HeightScaleMie) * Pm2) * Dw;
        }
    }
}

void main()
{
    float Mu, MuS, Nu;

    GetMuMuSNu(gl_FragCoord.xy, g_Radius, g_Dhdh, Mu, MuS, Nu);

    vec3 RayleighMie = vec3(0.0f);

    Inscatter(g_Radius, Mu, MuS, Nu, RayleighMie);

	out_Output = vec4(RayleighMie, 0.0f);
}

#endif // __INCLUDE_FS_INSCATTER_MULTIPLE_A_GLSL__