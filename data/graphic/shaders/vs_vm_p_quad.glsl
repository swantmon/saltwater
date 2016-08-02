
#ifndef __INCLUDE_VS_VM_P_QUAD_GLSL__
#define __INCLUDE_VS_VM_P_QUAD_GLSL__

#include "common_global.glsl"

layout(row_major, std140, binding = 1) uniform UB1
{
    mat4 m_ModelMatrix;
};

layout(location = 0) in vec2 VertexPosition;

layout(location = 2) out vec2 PSTexCoord;

out gl_PerVertex
{
    vec4 gl_Position;
};

void main()
{
	vec4 WSPosition = m_ModelMatrix * vec4(VertexPosition.xy, 0.0f, 1.0f);
    
    PSTexCoord = VertexPosition.xy;

	gl_Position = g_WorldToScreen * WSPosition;
}

#endif // __INCLUDE_VS_VM_P_QUAD_GLSL__