
#ifndef __INCLUDE_FS_INSCATTER_COPY_SINGLE_GLSL__
#define __INCLUDE_FS_INSCATTER_COPY_SINGLE_GLSL__

#include "scattering/scattering_common.glsl"
#include "scattering/scattering_precompute_common.glsl"

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
void main()
{
    vec3 UVW = vec3(gl_FragCoord.xy, in_Layer) / vec3(g_InscatterMuS * g_InscatterNu, g_InscatterMu, g_InscatterAltitude);

    vec3 Rayleigh = textureLod(g_DeltaSR, UVW, 0).rgb;
    vec3 Mie      = textureLod(g_DeltaSM, UVW, 0).rgb;

    out_Output = vec4(Rayleigh, Mie.r);
}

#endif // __INCLUDE_FS_INSCATTER_COPY_SINGLE_GLSL__