
#ifndef __INCLUDE_VS_M_PNTBX0_GLSL__
#define __INCLUDE_VS_M_PNTBX0_GLSL__

#include "common_global.glsl"

layout(std140, binding = 1) uniform UB1
{
    mat4 m_ModelMatrix;
};

layout(location = 0) in vec3 in_Vertex;
layout(location = 1) in vec3 in_Normal;
layout(location = 2) in vec3 in_Tangent;
layout(location = 3) in vec3 in_Bitangent;
layout(location = 4) in vec2 in_UV;

layout(location = 0) out vec3 out_WSPosition;
layout(location = 1) out vec3 out_WSNormal;
layout(location = 2) out vec2 out_UV;
layout(location = 3) out mat3 out_WSNormalMatrix;

out gl_PerVertex
{
    vec4 gl_Position;
};

void main(void)
{
    vec4 WSPosition  = m_ModelMatrix * vec4(in_Vertex   , 1.0f);
    vec4 WSNormal    = m_ModelMatrix * vec4(in_Normal   , 0.0f);
    vec4 WSTangent   = m_ModelMatrix * vec4(in_Tangent  , 0.0f);
    vec4 WSBitangent = m_ModelMatrix * vec4(in_Bitangent, 0.0f);
    
    mat3 WSNormalMatrix = mat3(WSTangent.xyz, WSBitangent.xyz, WSNormal.xyz);
    
    out_WSPosition     = WSPosition.xyz;
    out_WSNormal       = WSNormal.xyz;
    out_UV             = in_UV;
    out_WSNormalMatrix = WSNormalMatrix;
    
    gl_Position      = g_WorldToScreen * WSPosition;
}

#endif // __INCLUDE_VS_M_PNTBX0_GLSL__