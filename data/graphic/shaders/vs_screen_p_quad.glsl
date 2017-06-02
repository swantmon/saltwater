
#ifndef __INCLUDE_VS_SCREEN_P_QUAD_GLSL__
#define __INCLUDE_VS_SCREEN_P_QUAD_GLSL__

#include "common_global.glsl"

layout(location = 0) in vec2 in_VertexPosition;

layout(location = 2) out vec2 out_UV;

out gl_PerVertex
{
    vec4 gl_Position;
};

void main()
{
	vec4 Position = vec4(in_VertexPosition.xy, 0.0f, 1.0f);
    
    out_UV = vec2(Position.x, 1.0f - Position.y);

	gl_Position = g_WorldToQuad * Position;
}

#endif // __INCLUDE_VS_SCREEN_P_QUAD_GLSL__