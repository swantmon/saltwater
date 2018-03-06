
#ifndef __INCLUDE_VS_VM_P_QUAD_GLSL__
#define __INCLUDE_VS_VM_P_QUAD_GLSL__

#include "common_global.glsl"

layout(std140, binding = 1) uniform UB1
{
    mat4 m_ModelMatrix;
};

layout(location = 0) in vec2 in_Vertex;

layout(location = 2) out vec2 out_UV;

out gl_PerVertex
{
    vec4 gl_Position;
};

void main()
{
	vec4 WSPosition = m_ModelMatrix * vec4(in_Vertex.xy, 0.0f, 1.0f);
    
    out_UV = in_Vertex.xy;

	gl_Position = g_WorldToScreen * WSPosition;
}

#endif // __INCLUDE_VS_VM_P_QUAD_GLSL__