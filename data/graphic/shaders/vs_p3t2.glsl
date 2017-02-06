
#ifndef __INCLUDE_VS_NON_P_GLSL__
#define __INCLUDE_VS_NON_P_GLSL__

#include "common_global.glsl"

layout(location = 0) in vec3 in_Position;
layout(location = 1) in vec2 in_UV;

layout(location = 2) out vec2 out_UV;

out gl_PerVertex
{
    vec4 gl_Position;
};

void main(void)
{
	out_UV = in_UV;

    gl_Position = g_WorldToScreen * vec4(in_Position.xyz, 1.0f);
}

#endif // __INCLUDE_VS_NON_P_GLSL__