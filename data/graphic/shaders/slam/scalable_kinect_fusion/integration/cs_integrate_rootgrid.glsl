
#ifndef __INCLUDE_CS_KINECT_INTEGRATE_ROOTGRID_GLSL__
#define __INCLUDE_CS_KINECT_INTEGRATE_ROOTGRID_GLSL__

#include "slam/scalable_kinect_fusion/common_scalable.glsl"
#include "slam/scalable_kinect_fusion/common_indirect.glsl"

layout(std430, binding = 6) buffer Level1Queue
{
    uint g_VolumeID[];
};

layout(std430, binding = 7) buffer Indirect
{
    SIndirectBuffers g_Indirect;
};

layout (local_size_x = TILE_SIZE1D, local_size_y = 1, local_size_z = 1) in;
void main()
{
    if (gl_GlobalInvocationID.x < g_Indirect.m_Indexed.m_InstanceCount)
    {
        int VoxelIndex = int(g_VolumeID[gl_GlobalInvocationID.x]);

        // Check if voxel is already in hierarchy

        int RootGridItemIndex = g_CurrentVolumeIndex * VOXELS_PER_ROOTGRID + VoxelIndex;

        if (g_RootGridPool[RootGridItemIndex].m_PoolIndex == -1) // Is the voxel empty?
        {
            // Add voxel to pool and to root grid

            g_RootGridPool[RootGridItemIndex].m_PoolIndex = atomicAdd(g_Level1GridPoolItemCount, 1);
        }
    }
}

#endif // __INCLUDE_CS_KINECT_INTEGRATE_ROOTGRID_GLSL__