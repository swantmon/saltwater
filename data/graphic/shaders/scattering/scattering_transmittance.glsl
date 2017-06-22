
#ifndef __INCLUDE_FS_SCATTERING_TRANSMITTANCE_GLSL__
#define __INCLUDE_FS_SCATTERING_TRANSMITTANCE_GLSL__

#include "scattering/scattering_common.glsl"
#include "scattering/scattering_precompute_common.glsl"

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
float GetOpticalDepth(in float _ScaleHeight, in float _Radius, in float _Mu)
{
    // -----------------------------------------------------------------------------
    // Calculates the overall thickness of the atmosphere on a given path
    // -----------------------------------------------------------------------------
    float Result = 0.0f;

    float Dx = Limit(_Radius, _Mu) / float(g_TransmittanceSampleCount);
    float Xi = 0.0f;
    float Yi = exp(-(_Radius - g_RadiusGround) / _ScaleHeight);

    for (int i = 1; i <= g_TransmittanceSampleCount; ++ i) 
	{
        float Xj = float(i) * Dx;
        float Yj = exp(-(sqrt(_Radius * _Radius + Xj * Xj + 2.0f * Xj * _Radius * _Mu) - g_RadiusGround) / _ScaleHeight);

        Result += (Yi + Yj) / 2.0f * Dx;

        Xi = Xj;
        Yi = Yj;
    }

    return _Mu < -sqrt(1.0 - (g_RadiusGround / _Radius) * (g_RadiusGround / _Radius)) ? 1e9 : Result;
}

void main()
{
    // -----------------------------------------------------------------------------
    // Calculate transmittance
    // -----------------------------------------------------------------------------
    vec2 RMu = GetTransmittanceRMu(in_UV);
    
    float Radius = RMu.x;
    float Mu = RMu.y;

    vec3 OpticalDepth = g_BetaRayleigh * GetOpticalDepth(g_HeightScaleRayleigh, Radius, Mu) + g_BetaMieExtinction * GetOpticalDepth(g_HeightScaleMie, Radius, Mu);

	out_Output = vec4(exp(-OpticalDepth), 1.0f);
}

#endif // __INCLUDE_FS_SCATTERING_TRANSMITTANCE_GLSL__