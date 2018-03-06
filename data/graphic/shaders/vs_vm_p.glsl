
#ifndef __INCLUDE_VS_VM_P_GLSL__
#define __INCLUDE_VS_VM_P_GLSL__

layout(std140, binding = 0) uniform UB0
{
    mat4 m_ViewProjectionMatrix;
    mat4 m_ModelMatrix;
};

layout(location = 0) in vec3 in_Vertex;
layout(location = 1) in vec3 in_Normal;

layout(location = 0) out vec3 out_WSPosition;
layout(location = 1) out vec3 out_WSNormal;

out gl_PerVertex
{
    vec4 gl_Position;
};

void main(void)
{
    vec4 WSPosition = m_ModelMatrix * vec4(in_Vertex.xyz, 1.0f);
    vec4 WSNormal   = m_ModelMatrix * vec4(in_Normal.xyz  , 0.0f);
    
    out_WSPosition = WSPosition.xyz;
    out_WSNormal   = WSNormal.xyz;
    
    gl_Position = m_ViewProjectionMatrix * WSPosition;
}

#endif // __INCLUDE_VS_VM_P_GLSL__