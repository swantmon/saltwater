
#ifndef __INCLUDE_FS_IRRADIANCE_SINGLE_GLSL__
#define __INCLUDE_FS_IRRADIANCE_SINGLE_GLSL__

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
void main()
{
    // -----------------------------------------------------------------------------
    // Compute
    // -----------------------------------------------------------------------------
    vec2  RMuS   = GetIrradianceRMuS(gl_FragCoord.xy);
    float Radius = RMuS.x;
    float MuS    = RMuS.y;

    out_Output = vec4(GetTransmittance(Radius, MuS) * max(MuS, 0.0f), 0.0f);
}

#endif // __INCLUDE_FS_IRRADIANCE_SINGLE_GLSL__