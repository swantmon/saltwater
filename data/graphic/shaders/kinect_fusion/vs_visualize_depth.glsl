
#ifndef __INCLUDE_VS_KINECT_GLSL__
#define __INCLUDE_VS_KINECT_GLSL__

#include "common_global.glsl"

out gl_PerVertex
{
    vec4 gl_Position;
};

layout(location = 0) out vec2 out_TexCoord;

layout(row_major, std140, binding = 1) uniform PerDrawCall
{
	mat4 g_WorldMatrix;
	mat4 g_WorldToViewMatrix;
};

vec4 VertexData[3] =
{
	vec4(3.0, 1.0, 2.0, 0.0),
	vec4(-1.0, 1.0, 0.0, 0.0),
	vec4(-1.0, -3.0, 0.0, 2.0)
};

void main(void)
{
	out_TexCoord = VertexData[gl_VertexID].zw;
    gl_Position = vec4(VertexData[gl_VertexID].xy, 0.0, 1.0);
}

#endif // __INCLUDE_VS_KINECT_GLSL__