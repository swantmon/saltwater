
#ifndef __INCLUDE_VS_VM_PNX0_GLSL__
#define __INCLUDE_VS_VM_PNX0_GLSL__

layout(std140, binding = 0) uniform UB0
{
    mat4 m_ViewProjection;
};

layout(std140, binding = 1) uniform UB1
{
    mat4 m_ModelMatrix;
};

layout(location = 0) in vec3 in_Vertex;
layout(location = 1) in vec3 in_Normal;
layout(location = 4) in vec2 in_UV;

layout(location = 0) out vec3 out_WSPosition;
layout(location = 1) out vec3 out_WSNormal;
layout(location = 2) out vec2 out_UV;

out gl_PerVertex
{
    vec4 gl_Position;
};

void main(void)
{
    vec4 WSPosition = m_ModelMatrix * vec4(in_Vertex, 1.0f);
    vec4 WSNormal   = m_ModelMatrix * vec4(in_Normal, 0.0f);
    
    out_WSPosition = WSPosition.xyz;
    out_WSNormal   = WSNormal.xyz;
    out_UV         = in_UV;
    
    gl_Position = m_ViewProjection * WSPosition;
}

#endif // __INCLUDE_VS_VM_PNX0_GLSL__