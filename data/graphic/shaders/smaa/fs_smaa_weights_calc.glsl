
#ifndef __INCLUDE_FS_SMAA_WEIGHTS_CALC_GLSL__
#define __INCLUDE_FS_SMAA_WEIGHTS_CALC_GLSL__

#define SMAA_GLSL_4
#define SMAA_PRESET_ULTRA
#define SMAA_INCLUDE_PS 1
#include "smaa/smaa.glsl"

// -----------------------------------------------------------------------------
// Defines / Settings
// -----------------------------------------------------------------------------

layout(location = 0) in vec2 in_UV;
layout(location = 1) in vec4 in_Offset[3];
layout(location = 4) in vec2 in_PixCoord;

// -----------------------------------------------------------------------------
// Output to fragment
// -----------------------------------------------------------------------------
layout(location = 0) out vec4 out_Output;

layout(binding = 0) uniform sampler2D ps_EdgesTexture;
layout(binding = 1) uniform sampler2D ps_AreaTexture;
layout(binding = 2) uniform sampler2D ps_SearchTexture;

void main(void)
{
    out_Output = SMAABlendingWeightCalculationPS(in_UV, in_PixCoord, in_Offset, ps_EdgesTexture, ps_AreaTexture, ps_SearchTexture, vec4(0.0));
}

#endif // __INCLUDE_FS_SMAA_WEIGHTS_CALC_GLSL__