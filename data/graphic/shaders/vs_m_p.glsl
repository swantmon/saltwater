
#ifndef __INCLUDE_VS_M_P_GLSL__
#define __INCLUDE_VS_M_P_GLSL__

#include "common_global.glsl"

layout(std140, binding = 1) uniform UB1
{
    mat4 m_ModelMatrix;
};

layout(location = 0) in vec3 in_Vertex;

layout(location = 0) out vec3 out_WSPosition;

out gl_PerVertex
{
    vec4 gl_Position;
};

void main(void)
{
    vec4 WSPosition = m_ModelMatrix * vec4(in_Vertex.xyz, 1.0f);
    
    out_WSPosition = WSPosition.xyz;
    
    gl_Position = g_WorldToScreen * WSPosition;
}

#endif // __INCLUDE_VS_M_P_GLSL__