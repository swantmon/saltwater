
#ifndef __INCLUDE_FS_INSCATTER_MULTIPLE_B_GLSL__
#define __INCLUDE_FS_INSCATTER_MULTIPLE_B_GLSL__

#include "scattering/scattering_common.glsl"
#include "scattering/scattering_precompute_common.glsl"

// -----------------------------------------------------------------------------
// Input from engine
// -----------------------------------------------------------------------------
layout(row_major, std140, binding = 3) uniform PSLayerValues
{
    vec4  g_Dhdh;
    float g_Radius;
};


// -----------------------------------------------------------------------------
// Input to fragment from previous stage
// -----------------------------------------------------------------------------
layout(location = 2) in vec2 in_UV;
layout(location = 4) in flat uint in_Layer;

// -----------------------------------------------------------------------------
// Output to fragment
// -----------------------------------------------------------------------------
layout(location = 0) out vec4 out_Output;

// -----------------------------------------------------------------------------
// Main
// -----------------------------------------------------------------------------
vec3 Integrate(float _Radius, float _Mu, float _MuS, float _Nu, float _T)
{
    float Ri = sqrt(_Radius * _Radius + _T * _T + 2.0f * _Radius * _Mu * _T);
    float Mui = (_Radius * _Mu + _T) / Ri;
    float MuSi = (_Nu * _T + _MuS * _Radius) / Ri;

    return Texture4DSample(g_DeltaJ, Ri, Mui, MuSi, _Nu).rgb * GetTransmittance(_Radius, _Mu, _T);
}

vec3 Inscatter(float _Radius, float _Mu, float _MuS, float _Nu)
{
    vec3 RayleighMie = vec3(0.0f);
    float Dx = Limit(_Radius, _Mu) / float(g_InscatterIntegralSampleCount);
    float Xi = 0.0;
    vec3 RayleighMiei = Integrate(_Radius, _Mu, _MuS, _Nu, 0.0f);

    for (int i = 1; i <= g_InscatterIntegralSampleCount; ++ i)
    {
        float Xj = float(i) * Dx;
        vec3 RayleighMiej = Integrate(_Radius, _Mu, _MuS, _Nu, Xj);

        RayleighMie += (RayleighMiei + RayleighMiej) / 2.0f * Dx;
        Xi = Xj;
        RayleighMiei = RayleighMiej;
    }

    return RayleighMie;
}

void main()
{
    float Mu, MuS, Nu;

    GetMuMuSNu(gl_FragCoord.xy, g_Radius, g_Dhdh, Mu, MuS, Nu);

    out_Output = vec4(Inscatter(g_Radius, Mu, MuS, Nu), 1.0f);
}

#endif // __INCLUDE_FS_INSCATTER_MULTIPLE_B_GLSL__