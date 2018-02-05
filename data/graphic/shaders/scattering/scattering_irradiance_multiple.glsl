
#ifndef __INCLUDE_FS_IRRADIANCE_MULTIPLE_GLSL__
#define __INCLUDE_FS_IRRADIANCE_MULTIPLE_GLSL__

#include "scattering/scattering_common.glsl"
#include "scattering/scattering_precompute_common.glsl"

// -----------------------------------------------------------------------------
// Defines
// -----------------------------------------------------------------------------
const float g_DeltaPhi   = g_PI / float(g_IrradianceIntegralSampleCount);
const float g_DeltaTheta = g_PI / float(g_IrradianceIntegralSampleCount);

// -----------------------------------------------------------------------------
// Input from engine
// -----------------------------------------------------------------------------
layout(std140, binding = 4) uniform PSScatteringOrder
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
void main()
{
    // -----------------------------------------------------------------------------
    // Compute
    // -----------------------------------------------------------------------------
    vec2 RMuS = GetIrradianceRMuS(gl_FragCoord.xy);
    float Radius = RMuS.x;
    float MuS = RMuS.y;

    vec3 S = vec3(max(sqrt(1.0f - MuS * MuS), 0.0f), 0.0f, MuS);

    vec3 Result = vec3(0.0f);
    
    for (int iPhi = 0; iPhi < g_IrradianceIntegralSampleCount * 2; ++ iPhi)
    {
        float Phi = (float(iPhi) + 0.5f) * g_DeltaPhi;

        for (int iTheta = 0; iTheta < g_IrradianceIntegralSampleCount / 2; ++ iTheta)
        {
            float Theta = (float(iTheta) + 0.5f) * g_DeltaTheta;
            float Dw = g_DeltaTheta * g_DeltaPhi * sin(Theta);
            vec3 W = vec3(cos(Phi) * sin(Theta), sin(Phi) * sin(Theta), cos(Theta));
            float Nu = dot(S, W);

            if (g_FirstOrder == 1.0f)
            {
                float Pr1 = PhaseFunctionR(Nu);
                float Pm1 = PhaseFunctionM(Nu);

                vec3 Rayleigh1 = Texture4DSample(g_DeltaSR, Radius, W.z, MuS, Nu).rgb;
                vec3 Mie1      = Texture4DSample(g_DeltaSM, Radius, W.z, MuS, Nu).rgb;

                Result += (Rayleigh1 * Pr1 + Mie1 * Pm1) * W.z * Dw;
            }
            else
            {
                Result += Texture4DSample(g_DeltaSR, Radius, W.z, MuS, Nu).rgb * W.z * Dw;
            }
        }
    }

    out_Output = vec4(Result, 0.0f);
}

#endif // __INCLUDE_FS_IRRADIANCE_MULTIPLE_GLSL__