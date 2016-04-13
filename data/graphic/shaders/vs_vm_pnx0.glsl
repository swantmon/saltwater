
#ifndef __INCLUDE_VS_VM_PNX0_GLSL__
#define __INCLUDE_VS_VM_PNX0_GLSL__

layout(row_major, std140, binding = 0) uniform UConstantBufferVS0 /* UPerLightConstantBuffer */
{
    mat4 m_ViewProjection;
};

layout(row_major, std140, binding = 1) uniform UConstantBufferVS1 /* UPerDrawCallConstantBuffer */
{
    mat4 m_ModelMatrix;
};

layout(location = 0) in vec3 VertexPosition;
layout(location = 1) in vec3 VertexNormal;
layout(location = 4) in vec2 VertexTexCoord;

layout(location = 0) out vec3 PSPosition;
layout(location = 1) out vec3 PSNormal;
layout(location = 2) out vec2 PSTexCoord;

out gl_PerVertex
{
    vec4 gl_Position;
};

void main(void)
{
    vec4 WSPosition = m_ModelMatrix * vec4(VertexPosition, 1.0f);
    vec4 WSNormal   = m_ModelMatrix * vec4(VertexNormal  , 0.0f);
    
    PSPosition = WSPosition.xyz;
    PSNormal   = WSNormal.xyz;
    PSTexCoord = VertexTexCoord;
    
    gl_Position = m_ViewProjection * WSPosition;
}

#endif // __INCLUDE_VS_VM_PNX0_GLSL__