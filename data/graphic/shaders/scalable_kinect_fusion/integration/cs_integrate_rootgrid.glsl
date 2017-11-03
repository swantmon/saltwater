
#ifndef __INCLUDE_CS_KINECT_INTEGRATE_ROOTGRID_GLSL__
#define __INCLUDE_CS_KINECT_INTEGRATE_ROOTGRID_GLSL__

#include "scalable_kinect_fusion/common_scalable.glsl"
#include "scalable_kinect_fusion/common_indirect.glsl"

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

        int CurrentRootGridItemIndex = g_CurrentVolumeIndex * VOXELS_PER_ROOTGRID + VoxelIndex;
        SGridPoolItem RootGridItem = g_RootGridPool[CurrentRootGridItemIndex];

        if (RootGridItem.m_PoolIndex == -1) // Is the voxel empty?
        {
            // Add voxel to pool and to root grid

            int Level1PoolIndex = atomicAdd(g_Level1GridPoolItemCount, 1);

            RootGridItem.m_PoolIndex = Level1PoolIndex;
            g_RootGridPool[CurrentRootGridItemIndex] = RootGridItem;
        }
    }
}

#endif // __INCLUDE_CS_KINECT_INTEGRATE_ROOTGRID_GLSL__