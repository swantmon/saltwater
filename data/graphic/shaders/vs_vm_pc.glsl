
#ifndef __INCLUDE_VS_VM_PC_GLSL__
#define __INCLUDE_VS_VM_PC_GLSL__

layout(std140, binding = 0) uniform UB0
{
    mat4 m_ViewProjection;
};

layout(std140, binding = 1) uniform UB1
{
    mat4 m_ModelMatrix;
};

layout(location=0) in vec3 in_Vertex;
layout(location=1) in vec3 in_Color;

layout(location = 0) out vec3 out_WSPosition;
layout(location = 1) out vec3 out_Color;

out gl_PerVertex
{
    vec4 gl_Position;
};

void main(void)
{
    vec4 WSPosition = m_ModelMatrix * vec4(in_Vertex.xyz, 1.0f);
    
    out_WSPosition = WSPosition.xyz;
    out_Color      = in_Color;
    
    gl_Position = m_ViewProjection * WSPosition;
}

#endif // __INCLUDE_VS_VM_PC_GLSL__