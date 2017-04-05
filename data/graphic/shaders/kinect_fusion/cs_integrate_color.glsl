
#ifndef __INCLUDE_CS_KINECT_INTEGRATE_COLOR_GLSL__
#define __INCLUDE_CS_KINECT_INTEGRATE_COLOR_GLSL__

#include "common_tracking.glsl"

// -----------------------------------------------------------------------------
// Constants
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Input from engine
// -----------------------------------------------------------------------------

layout(binding = 0, rgba8) uniform image3D cs_ColorVolume;
layout(binding = 1, rgba8) readonly uniform image2D cs_Color;

// -------------------------------------------------------------------------------------
// Functions
// -------------------------------------------------------------------------------------

layout (local_size_x = TILE_SIZE2D, local_size_y = TILE_SIZE2D, local_size_z = 1) in;
void main()
{
    const int x = int(gl_GlobalInvocationID.x);
    const int y = int(gl_GlobalInvocationID.y);
    
    const vec3 CameraPosition = g_PoseMatrix[3].xyz;
    ivec3 VoxelCoords = ivec3(x, y, 0);

    
}

#endif // __INCLUDE_CS_KINECT_INTEGRATE_COLOR_GLSL__