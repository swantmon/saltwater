
#ifndef __INCLUDE_CS_KINECT_INTEGRATE_ROOTGRID_GLSL__
#define __INCLUDE_CS_KINECT_INTEGRATE_ROOTGRID_GLSL__

#include "scalable_kinect_fusion/common_scalable.glsl"

layout(std430, binding = 6) buffer Level1Queue
{
    uint g_VolumeID[];
};

layout (local_size_x = TILE_SIZE1D, local_size_y = 1, local_size_z = 1) in;
void main()
{
    if (gl_GlobalInvocationID.x < g_Level1QueueSize)
    {
        uint VoxelIndex = g_VolumeID[gl_GlobalInvocationID.x];

        // Add voxel to pool

        uint Level1PoolIndex = atomicAdd(g_Level1GridPoolItemCount, 1);

        // Add voxel in higher grid level

        uint RootGridPoolIndex = g_CurrentVolumeIndex * 16 * 16 * 16;
        RootGridPoolIndex += gl_GlobalInvocationID.x;

        RootGridPoolIndex = Level1PoolIndex;
    }
}

#endif // __INCLUDE_CS_KINECT_INTEGRATE_ROOTGRID_GLSL__