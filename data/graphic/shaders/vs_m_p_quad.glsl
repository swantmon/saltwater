
#ifndef __INCLUDE_VS_M_P_QUAD_GLSL__
#define __INCLUDE_VS_M_P_QUAD_GLSL__

#include "vs_global.glsl"

layout(row_major, std140, binding = 1) uniform UConstantBufferVS1 /* UPerDrawCallConstantBuffer */
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
	vec4 Position = vec4(VertexPosition.xy, 0.0f, 1.0f);
    
    PSTexCoord = vec2(Position.x, 1.0f - Position.y);

	gl_Position = vs_ViewProjectionScreenMatrix * m_ModelMatrix * Position;
}

#endif // __INCLUDE_VS_M_P_QUAD_GLSL__