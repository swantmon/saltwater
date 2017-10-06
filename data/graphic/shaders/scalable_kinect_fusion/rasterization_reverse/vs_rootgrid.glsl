
#ifndef __INCLUDE_VS_RASTERIZATION_ROOTGRID_GLSL__
#define __INCLUDE_VS_RASTERIZATION_ROOTGRID_GLSL__

#include "common_global.glsl"

layout(row_major, std140, binding = 1) uniform PerDrawCallData
{
    mat4 g_WorldMatrix;
    vec4 g_Color;
};

layout (binding = 0, rgba16f) uniform image2D cs_VertexMap;

out gl_PerVertex
{
    vec4 gl_Position;
};

// -----------------------------------------------------------------------------
// Functions
// -----------------------------------------------------------------------------

void main()
{
    ivec2 UV;
    UV.x = gl_VertexID % 512;
    UV.y = gl_VertexID / 512;
    vec4 WSPosition = imageLoad(cs_VertexMap, UV);
    WSPosition.w = 1.0f;
    WSPosition = g_WorldMatrix * WSPosition;
    gl_Position = g_WorldToScreen * WSPosition;
}

#endif // __INCLUDE_VS_RASTERIZATION_ROOTGRID_GLSL__