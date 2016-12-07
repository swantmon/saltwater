
#ifndef __INCLUDE_FS_SMAA_EDGE_DETECT_GLSL__
#define __INCLUDE_FS_SMAA_EDGE_DETECT_GLSL__

#define SMAA_GLSL_4
#define SMAA_PRESET_ULTRA
#define SMAA_INCLUDE_PS 1
#include "smaa.glsl"

// -----------------------------------------------------------------------------
// Textures
// -----------------------------------------------------------------------------

layout(binding = 0) uniform sampler2D ps_InputTexture;

layout(location = 0) in vec2 in_UV;
layout(location = 1) in vec4 in_Offset[3];

// -----------------------------------------------------------------------------
// Output to fragment
// -----------------------------------------------------------------------------
layout(location = 0) out vec4 out_Output;

void main(void)
{
	vec2 edge = SMAALumaEdgeDetectionPS(in_UV, in_Offset, ps_InputTexture);
    out_Output = vec4(edge, 0.0, 0.0);
}

#endif // __INCLUDE_FS_SMAA_EDGE_DETECT_GLSL__