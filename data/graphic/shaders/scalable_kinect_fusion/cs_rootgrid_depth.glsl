
#ifndef __INCLUDE_CS_ROOTGRID_DEPTH_GLSL__
#define __INCLUDE_CS_ROOTGRID_DEPTH_GLSL__

#include "scalable_kinect_fusion/common_tracking.glsl"

// -----------------------------------------------------------------------------
// Constants
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Input from engine
// -----------------------------------------------------------------------------

layout(row_major, std140, binding = 2) uniform UBODepthThreshold
{
    vec3 g_Offset;
};

layout(binding = 0, rg16f) uniform image3D cs_TSDFVolume;
layout(binding = 1, r16ui) readonly uniform uimage2D cs_Depth;
layout(binding = 0, offset = 0) uniform atomic_uint cs_Counter;

// -------------------------------------------------------------------------------------
// Functions
// -------------------------------------------------------------------------------------

layout (local_size_x = TILE_SIZE3D, local_size_y = TILE_SIZE3D, local_size_z = TILE_SIZE3D) in;
void main()
{
    atomicCounterIncrement(cs_Counter);
}

#endif // __INCLUDE_CS_ROOTGRID_DEPTH_GLSL__