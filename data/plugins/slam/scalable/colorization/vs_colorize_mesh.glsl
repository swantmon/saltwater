
#ifndef __INCLUDE_VS_OUTLINE_GLSL__
#define __INCLUDE_VS_OUTLINE_GLSL__

layout(std140, binding = 0) uniform PerDrawCallData
{
    mat4 g_WorldMatrix;
	vec4 g_Color;
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
    out_WSPosition = (g_WorldMatrix * vec4(in_VertexPosition, 1.0f)).xyz;
    gl_Position = vec4(in_TexCoord * 2.0f - 1.0f, 0.0f, 1.0f);
}

#endif // __INCLUDE_VS_OUTLINE_GLSL__