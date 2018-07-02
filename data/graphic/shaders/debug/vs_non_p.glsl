
#ifndef __INCLUDE_VS_NON_P_GLSL__
#define __INCLUDE_VS_NON_P_GLSL__

#include "common_global.glsl"

layout(location = 0) in vec3 in_Vertex;

out gl_PerVertex
{
    vec4 gl_Position;
};

void main(void)
{
    gl_Position = g_WorldToScreen * vec4(in_Vertex.xyz, 1.0f);
}

#endif // __INCLUDE_VS_NON_P_GLSL__