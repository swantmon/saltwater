
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

layout(location=0) in vec3 VertexPosition;
layout(location=1) in vec3 VertexColor;

layout(location = 0) out vec3 PSPosition;
layout(location = 1) out vec3 PSColor;

out gl_PerVertex
{
    vec4 gl_Position;
};

void main(void)
{
    vec4 WSPosition = m_ModelMatrix * vec4(VertexPosition.xyz, 1.0f);
    
    PSPosition = WSPosition.xyz;
    PSColor    = VertexColor;
    
    gl_Position = m_ViewProjection * WSPosition;
}

#endif // __INCLUDE_VS_VM_PC_GLSL__