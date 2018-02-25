
#ifndef __INCLUDE_CS_FIND_GARBAGE_GLSL__
#define __INCLUDE_CS_FIND_GARBAGE_GLSL__

#include "slam/scalable_kinect_fusion/common_scalable.glsl"
#include "slam/scalable_kinect_fusion/common_indirect.glsl"

// -----------------------------------------------------------------------------
// Buffers
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Compute Shader
// -----------------------------------------------------------------------------

const int g_Resolution = ROOT_RESOLUTION * LEVEL1_RESOLUTION;

layout (local_size_x = LEVEL2_RESOLUTION, local_size_y = LEVEL2_RESOLUTION, local_size_z = LEVEL2_RESOLUTION) in;
void main()
{
    uint Offset = gl_WorkGroupID.x * VOXELS_PER_LEVEL2GRID;
    int InnerOffset = OffsetToIndex(gl_LocalInvocationID, LEVEL2_RESOLUTION);
    
    g_TSDFPool[Offset + InnerOffset].m_TSDF = 0;

#ifdef CAPTURE_COLOR
    g_TSDFPool[Offset + InnerOffset].m_Color = 0;
#endif
}

#endif // __INCLUDE_CS_FIND_GARBAGE_GLSL__