
#ifndef __INCLUDE_FS_SMAA_BLENDING_GLSL__
#define __INCLUDE_FS_SMAA_BLENDING_GLSL__

#define SMAA_GLSL_4
#define SMAA_PRESET_ULTRA
#define SMAA_INCLUDE_PS 1
#include "smaa.glsl"

// -----------------------------------------------------------------------------
// Defines / Settings
// -----------------------------------------------------------------------------

layout(location = 0) in vec2 in_UV;
layout(location = 1) in vec4 in_Offset;

// -----------------------------------------------------------------------------
// Output to fragment
// -----------------------------------------------------------------------------
layout(location = 0) out vec4 out_Output;

layout(binding = 0) uniform sampler2D ps_InputTexture;
layout(binding = 2) uniform sampler2D ps_WeightsTexture;

void main(void)
{
    out_Output =  SMAANeighborhoodBlendingPS(in_UV, in_Offset, ps_InputTexture, ps_WeightsTexture);
}

#endif // __INCLUDE_FS_SMAA_BLENDING_GLSL__