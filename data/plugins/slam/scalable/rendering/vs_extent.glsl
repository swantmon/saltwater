
#ifndef __INCLUDE_VS_OUTLINE_GLSL__
#define __INCLUDE_VS_OUTLINE_GLSL__

#include "common_global.glsl"

layout(std140, binding = 1) uniform PerDrawCallData
{
    mat4 g_WorldMatrix;
	vec2 g_Offset;
	vec2 g_Size;
};

layout(location = 0) in vec2 in_VertexPosition;
layout(location = 1) in vec2 in_TexCoord;

out gl_PerVertex
{
    vec4 gl_Position;
};

layout(location = 0) out vec2 out_UV;

// -----------------------------------------------------------------------------
// Functions
// -----------------------------------------------------------------------------

void main()
{
	vec2 Position = in_VertexPosition.xy * 0.5f + 0.5f;
	Position = 1.0f - Position;
	Position *= g_Size;
	Position += g_Offset;
    vec4 WSPosition = g_WorldMatrix * vec4(Position.x, 0.0f, Position.y, 1.0f);
    out_UV = 1.0f - in_TexCoord;
    gl_Position = g_WorldToScreen * WSPosition;
}

#endif // __INCLUDE_VS_OUTLINE_GLSL__