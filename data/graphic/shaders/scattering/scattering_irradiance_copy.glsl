
#ifndef __INCLUDE_FS_IRRADIANCE_COPY_GLSL__
#define __INCLUDE_FS_IRRADIANCE_COPY_GLSL__

#include "scattering/scattering_common.glsl"
#include "scattering/scattering_precompute_common.glsl"

// -----------------------------------------------------------------------------
// Input from engine
// -----------------------------------------------------------------------------
layout(std140, binding = 5) uniform UB2
{
    float k; // wtf?
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
    // Copy texture
    // -----------------------------------------------------------------------------
    out_Output = k * textureLod(g_DeltaE, in_UV, 0.0f);
}

#endif // __INCLUDE_FS_IRRADIANCE_COPY_GLSL__