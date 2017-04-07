
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
layout(binding = 2, r16ui) readonly uniform uimage2D cs_Depth;

// -------------------------------------------------------------------------------------
// Functions
// -------------------------------------------------------------------------------------

layout (local_size_x = TILE_SIZE2D, local_size_y = TILE_SIZE2D, local_size_z = 1) in;
void main()
{
    const int x = int(gl_GlobalInvocationID.x);
    const int y = int(gl_GlobalInvocationID.y);
        
    const float Depth = imageLoad(cs_Depth, ivec2(x, y)).x / 1000.0f;
    
    vec2 CameraPlane = vec2(x, y);
    
    CameraPlane = (CameraPlane - g_Intrinisics[0].m_FocalPoint) * Depth / g_Intrinisics[0].m_FocalLength;

    vec3 VSPosition = vec3(CameraPlane, Depth);
    vec4 WSPosition = g_PoseMatrix * vec4(VSPosition, 1.0f);

    ivec3 VoxelCoords = ivec3(WSPosition.xyz / VOXEL_SIZE);

    vec3 Color = imageLoad(cs_Color, ivec2(DEPTH_IMAGE_WIDTH - x, y)).rgb;
    
    if (Color.r > 0.0f && Color.g > 0.0f && Color.b > 0.0f)
    {
        imageStore(cs_ColorVolume, VoxelCoords, vec4(Color, 1.0f));
    }
}

#endif // __INCLUDE_CS_KINECT_INTEGRATE_COLOR_GLSL__