
#ifndef __INCLUDE_VS_VM_P_GLSL__
#define __INCLUDE_VS_VM_P_GLSL__

layout(row_major, std140, binding = 0) uniform UB0
{
    mat4 m_ViewProjectionMatrix;
    mat4 m_ModelMatrix;
};

layout(location = 0) in vec3 VertexPosition;
layout(location = 1) in vec3 VertexNormal;

layout(location = 0) out vec3 PSPosition;
layout(location = 1) out vec3 PSNormal;

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
    
    gl_Position = m_ViewProjectionMatrix * WSPosition;
}

#endif // __INCLUDE_VS_VM_P_GLSL__