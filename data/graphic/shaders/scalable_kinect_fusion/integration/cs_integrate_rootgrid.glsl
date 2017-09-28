
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
    }

    
}

#endif // __INCLUDE_CS_KINECT_INTEGRATE_ROOTGRID_GLSL__