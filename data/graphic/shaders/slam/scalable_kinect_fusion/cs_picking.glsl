
#ifndef __INCLUDE_CS_PICKING_GLSL__
#define __INCLUDE_CS_PICKING_GLSL__

#include "slam/scalable_kinect_fusion/common_raycast.glsl"
// -----------------------------------------------------------------------------
// Constants
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Input from engine
// -----------------------------------------------------------------------------

layout(std430, binding = 7) buffer PickingBuffer
{
    vec4 g_RayStart;
    vec4 g_RayDirection;
    vec4 g_WorldHitPosition;
};

// -------------------------------------------------------------------------------------
// Functions
// -------------------------------------------------------------------------------------

layout (local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main()
{
    g_WorldHitPosition.xyz = GetPosition(g_RayStart.xyz, g_RayDirection.xyz);
}

#endif // __INCLUDE_CS_PICKING_GLSL__