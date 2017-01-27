
#ifndef __INCLUDE_CS_KINECT_INTEGRATE_VOLUME_GLSL__
#define __INCLUDE_CS_KINECT_INTEGRATE_VOLUME_GLSL__

#include "tracking_common.glsl"

// -----------------------------------------------------------------------------
// Constants
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Input from engine
// -----------------------------------------------------------------------------

layout(binding = 0, rg16ui) uniform uimage3D cs_Volume;
layout(binding = 1, r16ui) readonly uniform uimage2D cs_Depth;
layout(binding = 2, rgba32f) writeonly uniform image3D cs_Debug;

// -------------------------------------------------------------------------------------
// Functions
// -------------------------------------------------------------------------------------

layout (local_size_x = TILE_SIZE3D, local_size_y = TILE_SIZE3D, local_size_z = 1) in;
void main()
{
    const int x = int(gl_GlobalInvocationID.x);
    const int y = int(gl_GlobalInvocationID.y);
    
    const vec3 CameraPosition = vec3(0.5f, 0.5f, -0.5f);

    ivec3 VoxelPosition = ivec3(x, y, 0);

    for (VoxelPosition.z = 0; VoxelPosition.z < VOLUME_RESOLUTION; ++ VoxelPosition.z)
    {
        imageStore(cs_Volume, VoxelPosition, uvec4(0)); // clear for debugging

        vec3 WSVoxelPosition = (VoxelPosition - 0.5f) * VOXEL_SIZE;

        vec3 VSVoxelPosition = WSVoxelPosition - CameraPosition;

        // todo: Rotation

        vec3 CSVoxelPosition = mat3(g_KMatrix) * VSVoxelPosition;
        CSVoxelPosition.xyz /= CSVoxelPosition.z;

        if (CSVoxelPosition.x > 0 && CSVoxelPosition.x < DEPTH_IMAGE_WIDTH && CSVoxelPosition.y > 0 && CSVoxelPosition.y < DEPTH_IMAGE_HEIGHT)
        {
            const int Depth = int(imageLoad(cs_Depth, ivec2(CSVoxelPosition.xy)).x);

            const float DistanceFromCamera = length(CameraPosition - WSVoxelPosition);

            if (Depth != 0)
            {
                imageStore(cs_Debug, VoxelPosition, vec4(abs(DistanceFromCamera - Depth), DistanceFromCamera, Depth, 1.0f));

                if (abs(DistanceFromCamera - Depth / 1000.0f) < 0.001f)
                {
                    imageStore(cs_Volume, VoxelPosition, uvec4(1, 1, 0, 0));
                }
            }
            //imageStore(cs_Debug, ivec3(pos), depth == 0 ? vec4(0) : vec4(1));
        }
        //imageStore(cs_Debug, VoxelPosition, vec4(CSVoxelPosition, 1.0f));
    }
}

#endif // __INCLUDE_CS_KINECT_INTEGRATE_VOLUME_GLSL__