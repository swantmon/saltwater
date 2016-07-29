
#ifndef __INCLUDE_VS_M_PNTBX0_GLSL__
#define __INCLUDE_VS_M_PNTBX0_GLSL__

#include "common_global.glsl"

layout(row_major, std140, binding = 1) uniform UB1
{
    mat4 m_ModelMatrix;
};

layout(location = 0) in vec3 VertexPosition;
layout(location = 1) in vec3 VertexNormal;
layout(location = 2) in vec3 VertexTangent;
layout(location = 3) in vec3 VertexBitangent;
layout(location = 4) in vec2 VertexTexCoord;

layout(location = 0) out vec3 PSPosition;
layout(location = 1) out vec3 PSNormal;
layout(location = 2) out vec2 PSTexCoord;
layout(location = 3) out mat3 PSWSNormalMatrix;

out gl_PerVertex
{
    vec4 gl_Position;
};

void main(void)
{
    vec4 WSPosition  = m_ModelMatrix * vec4(VertexPosition , 1.0f);
    vec4 WSNormal    = m_ModelMatrix * vec4(VertexNormal   , 0.0f);
    vec4 WSTangent   = m_ModelMatrix * vec4(VertexTangent  , 0.0f);
    vec4 WSBitangent = m_ModelMatrix * vec4(VertexBitangent, 0.0f);
    
    mat3 WSNormalMatrix = mat3(WSTangent.xyz, WSBitangent.xyz, WSNormal.xyz);
    
    PSPosition       = WSPosition.xyz;
    PSNormal         = WSNormal.xyz;
    PSTexCoord       = VertexTexCoord;
    PSWSNormalMatrix = WSNormalMatrix;
    
    gl_Position      = ps_WorldToScreen * WSPosition;
}

#endif // __INCLUDE_VS_M_PNTBX0_GLSL__