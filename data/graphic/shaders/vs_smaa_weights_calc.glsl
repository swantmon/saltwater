
#ifndef __INCLUDE_VS_SMAA_WEIGHTS_CALC_GLSL__
#define __INCLUDE_VS_SMAA_WEIGHTS_CALC_GLSL__

#define SMAA_GLSL_4
#define SMAA_PRESET_ULTRA
#define SMAA_INCLUDE_PS 0
#include "smaa.glsl"

const vec4 VertexData[3] = vec4[3]
(
	vec4(-1.0, -1.0, 0.0, 0.0),
	vec4( 3.0, -1.0, 0.0, 2.0),
	vec4(-1.0,  3.0, 2.0, 0.0)
);

out gl_PerVertex
{
    vec4 gl_Position;
};

vec4 mad(vec4 x, vec4 y, vec4 z)
{
	return x * y + z;
}

layout(location = 0) out vec2 out_UV;
layout(location = 1) out vec4 out_Offset[3];
layout(location = 4) out vec2 out_PixCoord;

void main(void)
{
    gl_Position = vec4(VertexData[gl_VertexID].xy, 0.0, 1.0);
	out_UV = VertexData[gl_VertexID].xy * 0.5 + 0.5;
    SMAABlendingWeightCalculationVS(out_UV, out_PixCoord, out_Offset);
}

#endif // __INCLUDE_VS_SMAA_WEIGHTS_CALC_GLSL__