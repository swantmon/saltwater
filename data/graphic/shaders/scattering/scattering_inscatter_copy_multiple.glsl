
#ifndef __INCLUDE_FS_INSCATTER_COPY_MULTIPLE_GLSL__
#define __INCLUDE_FS_INSCATTER_COPY_MULTIPLE_GLSL__

#include "scattering/scattering_common.glsl"
#include "scattering/scattering_precompute_common.glsl"

// -----------------------------------------------------------------------------
// Input from engine
// -----------------------------------------------------------------------------
layout(std140, binding = 3) uniform PSLayerValues
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
layout(location = 0) out vec4 out_Output;

// -----------------------------------------------------------------------------
// Main
// -----------------------------------------------------------------------------
void main()
{
    float Mu, MuS, Nu;

    GetMuMuSNu(gl_FragCoord.xy, g_Radius, g_Dhdh, Mu, MuS, Nu);

    vec3 UVW = vec3(gl_FragCoord.xy, gl_Layer) / vec3(g_InscatterMuS * g_InscatterNu, g_InscatterMu, g_InscatterAltitude);

    out_Output = vec4(texture(g_DeltaSR, UVW).rgb / PhaseFunctionR(Nu), 0.0f);
}

#endif // __INCLUDE_FS_INSCATTER_COPY_MULTIPLE_GLSL__