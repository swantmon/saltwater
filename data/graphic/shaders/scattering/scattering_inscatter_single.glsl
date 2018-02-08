
#ifndef __INCLUDE_FS_INSCATTER_SINGLE_GLSL__
#define __INCLUDE_FS_INSCATTER_SINGLE_GLSL__

#include "scattering/scattering_common.glsl"
#include "scattering/scattering_precompute_common.glsl"

// -----------------------------------------------------------------------------
// Input from engine
// -----------------------------------------------------------------------------
layout(std140, binding = 3) uniform UB2
{
    vec4  g_Dhdh;
    float g_Radius;
};

// -----------------------------------------------------------------------------
// Input to fragment from previous stage
// -----------------------------------------------------------------------------
layout(location = 2) in vec2 in_UV;

// -----------------------------------------------------------------------------
// Output to fragment
// -----------------------------------------------------------------------------
layout(location = 0) out vec4 out_OutputRayleigh;
layout(location = 1) out vec4 out_OutputMie;

// -----------------------------------------------------------------------------
// Main
// -----------------------------------------------------------------------------
void Integrate(float _Radius, float _Mu, float _MuS, float _Nu, float _T, out vec3 _Rayleigh, out vec3 _Mie)
{
    _Rayleigh = vec3(0.0f);
    _Mie = vec3(0.0f);

    float Ri = sqrt(_Radius * _Radius + _T * _T + 2.0f * _Radius * _Mu * _T);
    float MuSi = (_Nu * _T + _MuS * _Radius) / Ri;

    Ri = max(g_RadiusGround, Ri);
    
    if (MuSi >= -sqrt(1.0f - g_RadiusGround * g_RadiusGround / (Ri * Ri)))
    {
        vec3 Ti = GetTransmittance(_Radius, _Mu, _T) * GetTransmittance(Ri, MuSi);
        _Rayleigh = exp(-(Ri - g_RadiusGround) / g_HeightScaleRayleigh) * Ti;
        _Mie = exp(-(Ri - g_RadiusGround) / g_HeightScaleMie) * Ti;
    }
}

void Inscatter(float _Radius, float _Mu, float _MuS, float _Nu, out vec3 _Rayleigh, out vec3 _Mie) // single scattering
{
    float Dx = Limit(_Radius, _Mu) / float(g_InscatterIntegralSampleCount);
    float Xi = 0.0f;
    vec3 Rayleighi = vec3(0.0f);
    vec3 Miei = vec3(0.0f);

    _Rayleigh = vec3(0.0f);
    _Mie = vec3(0.0f);

    Integrate(_Radius, _Mu, _MuS, _Nu, 0.0f, Rayleighi, Miei);

    #pragma unroll
    for (int i = 1; i <= g_InscatterIntegralSampleCount; ++ i)
    {
        float Xj = float(i) * Dx;
        vec3 Rayleighj = vec3(0.0f);
        vec3 Miej = vec3(0.0f);
        Integrate(_Radius, _Mu, _MuS, _Nu, Xj, Rayleighj, Miej);
        _Rayleigh += (Rayleighi + Rayleighj) / 2.0f * Dx;
        _Mie += (Miei + Miej) / 2.0f * Dx;
        Xi = Xj;
        Rayleighi = Rayleighj;
        Miei = Miej;
    }

    _Rayleigh *= g_BetaRayleigh;
    _Mie *= g_BetaMie;
}

void main()
{
    vec3 Rayleigh = vec3(0.0f);
    vec3 Mie = vec3(0.0f);

    float Mu, MuS, Nu;

    GetMuMuSNu(gl_FragCoord.xy, g_Radius, g_Dhdh, Mu, MuS, Nu);
    Inscatter(g_Radius, Mu, MuS, Nu, Rayleigh, Mie);

    out_OutputRayleigh = vec4(Rayleigh, 1.0f);
    out_OutputMie      = vec4(Mie, 1.0f);
}

#endif // __INCLUDE_FS_INSCATTER_SINGLE_GLSL__