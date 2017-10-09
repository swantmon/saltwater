
#ifndef __INCLUDE_VS_RASTERIZATION_ROOTGRID_GLSL__
#define __INCLUDE_VS_RASTERIZATION_ROOTGRID_GLSL__

#include "common_global.glsl"

// -----------------------------------------------------------------------------
// Buffers
// -----------------------------------------------------------------------------

layout(row_major, std140, binding = 0) uniform PerVolumeData
{
    ivec3 g_Offset;
    int g_Resolution;
};

layout (binding = 0, rgba16f) uniform image2D cs_VertexMap;

// -----------------------------------------------------------------------------
// Output
// -----------------------------------------------------------------------------

layout(location = 0) out vec3 out_WSPosition;

// -----------------------------------------------------------------------------
// Functions
// -----------------------------------------------------------------------------

void main()
{
    ivec2 UV;
    UV.x = gl_VertexID % 512;
    UV.y = gl_VertexID / 512;
    out_WSPosition = imageLoad(cs_VertexMap, UV);
    out_WSPosition.w = 1.0f;
    out_WSPosition = g_WorldMatrix * WSPosition;
}

#endif // __INCLUDE_VS_RASTERIZATION_ROOTGRID_GLSL__