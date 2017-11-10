
#ifndef __INCLUDE_CS_KINECT_INTEGRATE_LEVEL1_GLSL__
#define __INCLUDE_CS_KINECT_INTEGRATE_LEVEL1_GLSL__

#include "scalable_kinect_fusion/common_scalable.glsl"
#include "scalable_kinect_fusion/common_indirect.glsl"

layout(std430, binding = 6) buffer Level2Queue
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
        int VoxelLevel1Index = int(g_VolumeID[gl_GlobalInvocationID.x]);

        ivec3 Level1GridOffset = ivec3(IndexToOffset(VoxelLevel1Index, 16 * 8));
        ivec3 RootGridOffset = Level1GridOffset / 8;

        int VoxelRootGridIndex = OffsetToIndex(RootGridOffset, 16);

        int CurrentRootGridItemIndex = g_CurrentVolumeIndex * VOXELS_PER_ROOTGRID + VoxelRootGridIndex;
        SGridPoolItem RootGridItem = g_RootGridPool[CurrentRootGridItemIndex];

        int CurrentLevel1GridItemIndex = RootGridItem.m_PoolIndex + CurrentRootGridItemIndex;
        SGridPoolItem Level1GridItem = g_Level1GridPool[CurrentLevel1GridItemIndex];

        if (Level1GridItem.m_PoolIndex == -1) // Is the voxel empty?
        {
            // Add voxel to pool and to root grid

            int Level2PoolIndex = atomicAdd(g_TSDFPoolItemCount, 1);

            Level1GridItem.m_PoolIndex = Level2PoolIndex;
            g_Level1GridPool[CurrentRootGridItemIndex] = Level1GridItem;
        }
    }
}

#endif // __INCLUDE_CS_KINECT_INTEGRATE_LEVEL1_GLSL__