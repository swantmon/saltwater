
#ifndef __INCLUDE_CS_KINECT_INTEGRATE_VOLUME_GLSL__
#define __INCLUDE_CS_KINECT_INTEGRATE_VOLUME_GLSL__

#include "slam/scalable_kinect_fusion/common_raycast.glsl"

// -----------------------------------------------------------------------------
// Constants
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Input from engine
// -----------------------------------------------------------------------------

layout(std140, binding = 0) uniform PlaneData
{
    vec4 g_PlanePosition;
    vec2 g_Extent;
    ivec2 g_PlaneResolution;
    vec2 g_PixelSize;
};

layout (binding = 0, rgba8) uniform image2D cs_Plane;

// -------------------------------------------------------------------------------------
// Functions
// -------------------------------------------------------------------------------------

layout (local_size_x = TILE_SIZE2D, local_size_y = TILE_SIZE2D, local_size_z = 1) in;
void main()
{
    ivec2 PixelOffset = ivec2(gl_GlobalInvocationID.xy) - (g_PlaneResolution / 2);
    vec2 CameraOffset = g_PlanePosition.xy + PixelOffset * g_PixelSize;

    vec3 RayDirection = vec3(0.0f, 0.0f, -1.0f);
 
    RayDirection.x = RayDirection.x == 0.0f ? 1e-15f : RayDirection.x;
    RayDirection.y = RayDirection.y == 0.0f ? 1e-15f : RayDirection.y;
    RayDirection.z = RayDirection.z == 0.0f ? 1e-15f : RayDirection.z;

    vec3 CameraPosition = vec3(CameraOffset, g_PlanePosition.z + 0.3f);

    vec3 WSPosition, Color;

    GetPositionAndColor(CameraPosition, RayDirection, WSPosition, Color);
 
    if (WSPosition.x != 0.0f && (Color.r != 0.0f || Color.g != 0.0f || Color.b != 0.0f))
    {
        imageStore(cs_Plane, ivec2(gl_GlobalInvocationID.xy), vec4(Color, 1.0f));
    }
    else
    {
        imageStore(cs_Plane, ivec2(gl_GlobalInvocationID.xy), vec4(0.0f));
    }
}

#endif // __INCLUDE_CS_KINECT_INTEGRATE_VOLUME_GLSL__