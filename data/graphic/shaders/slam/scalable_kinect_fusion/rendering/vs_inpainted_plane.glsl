
#ifndef __INCLUDE_VS_OUTLINE_GLSL__
#define __INCLUDE_VS_OUTLINE_GLSL__

#include "common_global.glsl"

layout(std140, binding = 1) uniform PerDrawCallData
{
    mat4 g_WorldMatrix;
    vec4 g_Color;
};

layout(location = 0) in vec2 in_VertexPosition;
layout(location = 1) in vec2 in_TexCoord;

layout(location = 0) out vec2 out_TexCoord;

out gl_PerVertex
{
    vec4 gl_Position;
};

// -----------------------------------------------------------------------------
// Functions
// -----------------------------------------------------------------------------

void main()
{
    out_TexCoord = in_TexCoord;
    gl_Position = g_WorldToScreen * g_WorldMatrix * vec4(in_VertexPosition, 0.0f, 1.0f);
}

#endif // __INCLUDE_VS_OUTLINE_GLSL__