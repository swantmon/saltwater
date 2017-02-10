
#ifndef __INCLUDE_CS_KINECT_INTEGRATE_VOLUME_GLSL__
#define __INCLUDE_CS_KINECT_INTEGRATE_VOLUME_GLSL__

#include "tracking_common.glsl"

// -----------------------------------------------------------------------------
// Constants
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Input from engine
// -----------------------------------------------------------------------------

layout(binding = 0, rg16i) uniform iimage3D cs_Volume;
layout(binding = 1, r16ui) readonly uniform uimage2D cs_Depth;

// -------------------------------------------------------------------------------------
// Functions
// -------------------------------------------------------------------------------------

layout (local_size_x = TILE_SIZE3D, local_size_y = TILE_SIZE3D, local_size_z = 1) in;
void main()
{
    const int x = int(gl_GlobalInvocationID.x);
    const int y = int(gl_GlobalInvocationID.y);
    
    const vec3 CameraPosition = g_PoseMatrix[3].xyz;
    ivec3 VoxelCoords = ivec3(x, y, 0);

    for (VoxelCoords.z = 0; VoxelCoords.z < VOLUME_RESOLUTION; ++ VoxelCoords.z)
    {
        vec3 WSVoxelPosition = (VoxelCoords - 0.5f) * VOXEL_SIZE;

        vec3 VSVoxelPosition = (g_InvPoseMatrix * vec4(WSVoxelPosition, 1.0f)).xyz;

        vec3 CSVoxelPosition = mat3(g_Intrinisics[0].m_KMatrix) * VSVoxelPosition;
        CSVoxelPosition.xyz /= CSVoxelPosition.z;

        if (CSVoxelPosition.x > 0 && CSVoxelPosition.x < DEPTH_IMAGE_WIDTH && CSVoxelPosition.y > 0 && CSVoxelPosition.y < DEPTH_IMAGE_HEIGHT)
        {
            const int Depth = int(imageLoad(cs_Depth, ivec2(CSVoxelPosition.xy)).x);
            
            if (Depth != 0)
            {
                const vec2 LambdaPoint = (CSVoxelPosition.xy - g_Intrinisics[0].m_FocalPoint) * g_Intrinisics[0].m_InvFocalLength;
                const float Lambda = length(vec3(LambdaPoint, 1.0f));

                const float SDF = Depth - 1000.0f * length(CameraPosition - WSVoxelPosition) / Lambda;

                const float TSDF = clamp(SDF / TRUNCATED_DISTANCE, -1.0f, 1.0f);

                if (SDF >= -TRUNCATED_DISTANCE)
                {
                    vec2 Voxel = imageLoad(cs_Volume, VoxelCoords).xy;
                    Voxel.x /= INT16_MAX;

                    Voxel.x = (Voxel.x * Voxel.y + TSDF) / (Voxel.y + 1.0f);
                    Voxel.y = min(MAX_INTEGRATION_WEIGHT, Voxel.y + 1);

                    Voxel.x *= INT16_MAX;
                    imageStore(cs_Volume, VoxelCoords, ivec4(Voxel, 0, 0));
                }
            }
        }
    }
}

#endif // __INCLUDE_CS_KINECT_INTEGRATE_VOLUME_GLSL__