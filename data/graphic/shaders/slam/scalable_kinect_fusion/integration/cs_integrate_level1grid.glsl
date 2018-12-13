
#ifndef __INCLUDE_CS_KINECT_INTEGRATE_LEVEL1_GLSL__
#define __INCLUDE_CS_KINECT_INTEGRATE_LEVEL1_GLSL__

#include "slam/scalable_kinect_fusion/common_scalable.glsl"
#include "slam/scalable_kinect_fusion/common_indirect.glsl"

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
        
        ivec3 VoxelLevel1Offset = ivec3(IndexToOffset(VoxelLevel1Index, ROOT_RESOLUTION * LEVEL1_RESOLUTION));
        ivec3 VoxelRootOffset = VoxelLevel1Offset / LEVEL1_RESOLUTION;
        ivec3 VoxelLevel1InnerOffset = VoxelLevel1Offset % LEVEL1_RESOLUTION;
        
        int RootGridBufferOffset = g_CurrentVolumeIndex * VOXELS_PER_ROOTGRID;
        RootGridBufferOffset += OffsetToIndex(VoxelRootOffset, ROOT_RESOLUTION);
        
        int Level1GridBufferOffset = g_RootGridPool[RootGridBufferOffset].m_PoolIndex * VOXELS_PER_LEVEL1GRID;
        Level1GridBufferOffset += OffsetToIndex(VoxelLevel1InnerOffset, LEVEL1_RESOLUTION);
        
        if (g_Level1GridPool[Level1GridBufferOffset].m_PoolIndex == -1)
        {
            g_Level1GridPool[Level1GridBufferOffset].m_PoolIndex = atomicAdd(g_TSDFPoolItemCount, 1);
        }
        else
        {
            int Weight = g_Level1GridPool[Level1GridBufferOffset].m_Weight;
            atomicMax(g_RootGridPool[RootGridBufferOffset].m_Weight, Weight);
        }
    }
}

#endif // __INCLUDE_CS_KINECT_INTEGRATE_LEVEL1_GLSL__