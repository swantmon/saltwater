
#ifndef __INCLUDE_FS_SMAA_GLSL__
#define __INCLUDE_FS_SMAA_GLSL__

#include "common_global.glsl"

#define SMAA_RT_METRICS vec4(1.0 / 1280.0, 1.0 / 720.0, 1280.0, 720.0)
#define SMAA_GLSL_4
#define SMAA_PRESET_HIGH
#include "smaa.glsl"

// -----------------------------------------------------------------------------
// Textures
// -----------------------------------------------------------------------------

layout(binding = 0) uniform sampler2D ps_InputTexture;
layout(binding = 1) uniform sampler2D ps_AreaTexture;
layout(binding = 2) uniform sampler2D ps_SearchTexture;

layout(location = 0) in vec2 in_UV;
layout(location = 1) in vec4 offset[3];

// -----------------------------------------------------------------------------
// Output to fragment
// -----------------------------------------------------------------------------
layout(location = 0) out vec4 out_Output;

void main(void)
{
	vec2 edge = SMAALumaEdgeDetectionPS(in_UV, offset, ps_InputTexture);
    out_Output = vec4(edge, 0.0, 0.0);
}

#endif // __INCLUDE_FS_SMAA_GLSL__