
#ifndef __INCLUDE_VS_SMAA_GLSL__
#define __INCLUDE_VS_SMAA_GLSL__

#define SMAA_RT_METRICS vec4(1.0 / 1280.0, 1.0 / 720.0, 1280.0, 720.0)
#define SMAA_GLSL_4
#define SMAA_PRESET_HIGH

const vec4 Vertices[3] = vec4[3]
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
layout(location = 1) out vec4 offset[3];

void SMAAEdgeDetectionVS(vec2 texcoord, out vec4 offset[3])
{
    offset[0] = mad(SMAA_RT_METRICS.xyxy, vec4(-1.0, 0.0, 0.0, -1.0), texcoord.xyxy);
    offset[1] = mad(SMAA_RT_METRICS.xyxy, vec4( 1.0, 0.0, 0.0,  1.0), texcoord.xyxy);
    offset[2] = mad(SMAA_RT_METRICS.xyxy, vec4(-2.0, 0.0, 0.0, -2.0), texcoord.xyxy);
}

void main(void)
{
    gl_Position = vec4(Vertices[gl_VertexID].xy, 0.0, 1.0);
	out_UV = Vertices[gl_VertexID].xy * 0.5 + 0.5;
    SMAAEdgeDetectionVS(out_UV, offset);
}

#endif // __INCLUDE_VS_SMAA_GLSL__