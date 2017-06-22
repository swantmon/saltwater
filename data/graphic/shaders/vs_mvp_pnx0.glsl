
#ifndef __INCLUDE_VS_MVP_PNX0_GLSL__
#define __INCLUDE_VS_MVP_PNX0_GLSL__

layout(row_major, std140, binding = 0) uniform UB0
{
    mat4 m_ProjectionMatrix;
    mat4 m_ViewMatrix;
};

layout(row_major, std140, binding = 1) uniform UB1
{
    mat4 m_ModelMatrix;
};

layout(location = 0) in vec3 in_Position;
layout(location = 1) in vec3 in_Normal;
layout(location = 2) in vec2 in_UV;

layout(location = 0) out vec3 out_Position;
layout(location = 1) out vec3 out_Normal;
layout(location = 2) out vec2 out_UV;

out gl_PerVertex
{
    vec4 gl_Position;
};

void main(void)
{
    vec4 WSPosition = m_ModelMatrix * vec4(in_Position.xyz, 1.0f);
    vec4 WSNormal   = m_ModelMatrix * vec4(in_Normal.xyz  , 0.0f);
    
    out_Position = WSPosition.xyz;
    out_Normal   = WSNormal.xyz;
    out_UV       = in_UV;
    
    gl_Position = m_ProjectionMatrix * m_ViewMatrix * WSPosition;
}

#endif // __INCLUDE_VS_MVP_PNX0_GLSL__