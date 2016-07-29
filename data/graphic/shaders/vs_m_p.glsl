
#ifndef __INCLUDE_VS_M_P_GLSL__
#define __INCLUDE_VS_M_P_GLSL__

#include "common_global.glsl"

layout(row_major, std140, binding = 1) uniform UB1
{
    mat4 m_ModelMatrix;
};

layout(location = 0) in vec3 VertexPosition;

layout(location = 0) out vec3 PSPosition;

out gl_PerVertex
{
    vec4 gl_Position;
};

void main(void)
{
    vec4 WSPosition = m_ModelMatrix * vec4(VertexPosition.xyz, 1.0f);
    
    PSPosition = WSPosition.xyz;
    
    gl_Position = ps_WorldToScreen * WSPosition;
}

#endif // __INCLUDE_VS_M_P_GLSL__