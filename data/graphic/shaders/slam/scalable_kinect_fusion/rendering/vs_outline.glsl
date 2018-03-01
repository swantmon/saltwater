
#ifndef __INCLUDE_VS_OUTLINE_GLSL__
#define __INCLUDE_VS_OUTLINE_GLSL__

#include "common_global.glsl"

layout(std140, binding = 1) uniform PerDrawCallData
{
    mat4 g_WorldMatrix;
	vec4 g_Color;
};

layout(location = 0) in vec3 in_VertexPosition;

out gl_PerVertex
{
    vec4 gl_Position;
};

// -----------------------------------------------------------------------------
// Functions
// -----------------------------------------------------------------------------

void main()
{
    mat4 Rot = mat4(
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, -1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    );

    vec4 WSPosition = Rot * g_WorldMatrix * vec4(in_VertexPosition, 1.0f);
    gl_Position = g_WorldToScreen * WSPosition;
}

#endif // __INCLUDE_VS_OUTLINE_GLSL__