
#ifndef __INCLUDE_VS_RASTERIZATION_ROOTGRID_GLSL__
#define __INCLUDE_VS_RASTERIZATION_ROOTGRID_GLSL__

#include "common_global.glsl"

// -----------------------------------------------------------------------------
// Buffers
// -----------------------------------------------------------------------------

layout(std140, binding = 3) uniform PerVolumeData
{
    ivec3 g_Offset;
    int g_BufferOffset;
};

layout(location = 0) out flat int out_VertexID;

// -----------------------------------------------------------------------------
// Functions
// -----------------------------------------------------------------------------

void main()
{
    out_VertexID = gl_VertexID;
}

#endif // __INCLUDE_VS_RASTERIZATION_ROOTGRID_GLSL__