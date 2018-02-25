
#ifndef __INCLUDE_CS_FIND_GARBAGE_GLSL__
#define __INCLUDE_CS_FIND_GARBAGE_GLSL__

#include "slam/scalable_kinect_fusion/common_scalable.glsl"
#include "slam/scalable_kinect_fusion/common_indirect.glsl"

// -----------------------------------------------------------------------------
// Buffers
// -----------------------------------------------------------------------------

layout(std430, binding = 6) buffer Garbage
{
    int g_GarbageCount;
    int g_GarbageList[];
};

// -----------------------------------------------------------------------------
// Compute Shader
// -----------------------------------------------------------------------------

const int g_Resolution = ROOT_RESOLUTION * LEVEL1_RESOLUTION;

layout (local_size_x = LEVEL1_RESOLUTION, local_size_y = LEVEL1_RESOLUTION, local_size_z = LEVEL1_RESOLUTION) in;
void main()
{
    int RootGridBufferOffset = g_CurrentVolumeIndex * VOXELS_PER_ROOTGRID;
    RootGridBufferOffset += OffsetToIndex(gl_WorkGroupID, ROOT_RESOLUTION);
    
    int Level1GridBufferOffset = g_RootGridPool[RootGridBufferOffset].m_PoolIndex * VOXELS_PER_LEVEL1GRID;
    Level1GridBufferOffset += OffsetToIndex(gl_LocalInvocationID, LEVEL1_RESOLUTION);

    int Level2GridBufferOffset = g_Level1GridPool[Level1GridBufferOffset].m_PoolIndex * VOXELS_PER_LEVEL2GRID;

    if (Level2GridBufferOffset > -1)
    {
        if (g_Level1GridPool[Level1GridBufferOffset].m_Weight < 15)
        {
            g_Level1GridPool[Level1GridBufferOffset].m_PoolIndex = -1;
            g_Level1GridPool[Level1GridBufferOffset].m_Weight = 0;

            int GarbageIndex = atomicAdd(g_GarbageCount, 1);
            g_GarbageList[GarbageIndex] = Level2GridBufferOffset;
        }
    }
}

#endif // __INCLUDE_CS_FIND_GARBAGE_GLSL__