
#ifndef __INCLUDE_VS_M_PNX0_GLSL__
#define __INCLUDE_VS_M_PNX0_GLSL__

#include "common_global.glsl"

layout(row_major, std140, binding = 1) uniform UB1
{
    mat4 m_ModelMatrix;
};

layout(location = 0) in vec3 VertexPosition;
layout(location = 1) in vec3 VertexNormal;
layout(location = 2) in vec2 VertexTexCoord;

layout(location = 0) out vec3 PSPosition;
layout(location = 1) out vec3 PSNormal;
layout(location = 2) out vec2 PSTexCoord;

out gl_PerVertex
{
    vec4 gl_Position;
};

void main(void)
{
    vec4 WSPosition = m_ModelMatrix * vec4(VertexPosition.xyz, 1.0f);
    vec4 WSNormal   = m_ModelMatrix * vec4(VertexNormal.xyz  , 0.0f);
    
    PSPosition = WSPosition.xyz;
    PSNormal   = WSNormal.xyz;
    PSTexCoord = VertexTexCoord;
    
    gl_Position = g_WorldToScreen * WSPosition;
}

#endif // __INCLUDE_VS_M_PNX0_GLSL__