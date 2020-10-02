
#ifndef __INCLUDE_VS_COLORIZE_EXTENT__
#define __INCLUDE_VS_COLORIZE_EXTENT__

layout(std140, binding = 0) uniform PerDrawCallData
{
    mat4 g_WorldMatrix;
	vec2 g_Offset;
    vec2 g_Size;
	vec3 g_Normal;
};

layout(location = 0) in vec3 in_VertexPosition;
layout(location = 1) in vec2 in_TexCoord;

out gl_PerVertex
{
    vec4 gl_Position;
};

layout(location = 0) out vec3 out_WSPosition;

// -----------------------------------------------------------------------------
// Functions
// -----------------------------------------------------------------------------

void main()
{
    vec2 Position = in_VertexPosition.xz * 0.5f + 0.5f;
	Position = 1.0f - Position;
	Position *= g_Size;
	Position += g_Offset;
    out_WSPosition = (g_WorldMatrix * vec4(Position.x, 0.0f, Position.y, 1.0f)).xyz;
    gl_Position = vec4(in_TexCoord * 2.0f - 1.0f, 0.0f, 1.0f);
}

#endif // __INCLUDE_VS_COLORIZE_EXTENT__