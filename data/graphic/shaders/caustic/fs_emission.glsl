
#ifndef __INCLUDE_FS_CAUSTIC_EMISSION_GLSL__
#define __INCLUDE_FS_CAUSTIC_EMISSION_GLSL__

// -----------------------------------------------------------------------------
// Input to fragment from VS
// -----------------------------------------------------------------------------
layout(location = 0) in vec3 in_WSPosition;
layout(location = 1) in vec3 in_WSNormal;
layout(location = 2) in vec2 in_UV;

// -----------------------------------------------------------------------------
// Output to fragment
// -----------------------------------------------------------------------------
layout(location = 0) out vec4 out_Flux;
layout(location = 1) out vec4 out_PhotonLocation;

// -----------------------------------------------------------------------------
// Function
// -----------------------------------------------------------------------------
void main(void)
{
    out_PhotonLocation = vec4(in_WSPosition, 1.0f);
    out_Flux           = vec4(1.0f, 0.0f, 0.0f, 1.0f);
}

#endif // __INCLUDE_FS_CAUSTIC_EMISSION_GLSL__