
#ifndef __INCLUDE_FS_LIGHTPROBE_GLSL__
#define __INCLUDE_FS_LIGHTPROBE_GLSL__

#include "common.glsl"

layout(binding = 0) uniform samplerCube PSEnvironmentTexture;

// -----------------------------------------------------------------------------
// Input to fragment from VS
// -----------------------------------------------------------------------------
layout(location = 1) in vec3 in_Normal;

// -----------------------------------------------------------------------------
// Output to fragment
// -----------------------------------------------------------------------------
layout(location = 0) out vec4 out_Output;

// -----------------------------------------------------------------------------
// Main
// -----------------------------------------------------------------------------
void main(void)
{
    vec4 FinalColor = textureLod(PSEnvironmentTexture, in_Normal, 0);

    out_Output = vec4(clamp(FinalColor.xyz, 0.0f, F16_MAX), 1.0f);
}

#endif // __INCLUDE_FS_LIGHTPROBE_GLSL__