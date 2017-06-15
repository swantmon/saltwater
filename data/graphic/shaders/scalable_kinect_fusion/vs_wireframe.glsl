
#ifndef __INCLUDE_VS_WIREFRAME_GLSL__
#define __INCLUDE_VS_WIREFRAME_GLSL__

#include "common_global.glsl"

layout(row_major, std140, binding = 1) uniform PerDrawCallData
{
    mat4 g_WorldMatrix;
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
    vec4 WSPosition = g_WorldMatrix * vec4(in_VertexPosition, 1.0f);
    gl_Position = g_WorldToScreen * WSPosition;
}

#endif // __INCLUDE_VS_WIREFRAME_GLSL__