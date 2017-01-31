
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
    
    const vec3 CameraPosition = g_PoseMatrix[3].xyz;
    ivec3 VoxelCoords = ivec3(x, y, 0);

    for (VoxelCoords.z = 0; VoxelCoords.z < VOLUME_RESOLUTION; ++ VoxelCoords.z)
    {
        imageStore(cs_Volume, VoxelCoords, uvec4(0)); // clear for debugging

        vec3 WSVoxelPosition = (VoxelCoords - 0.5f) * VOXEL_SIZE;

        vec3 VSVoxelPosition = mat3(g_InvPoseRotationMatrix) * (WSVoxelPosition - CameraPosition); // todo: check if rotation is correct

        // todo: Rotation

        vec3 CSVoxelPosition = mat3(g_KMatrix) * VSVoxelPosition;
        CSVoxelPosition.xyz /= CSVoxelPosition.z;

        if (CSVoxelPosition.x > 0 && CSVoxelPosition.x < DEPTH_IMAGE_WIDTH && CSVoxelPosition.y > 0 && CSVoxelPosition.y < DEPTH_IMAGE_HEIGHT)
        {
            const int Depth = int(imageLoad(cs_Depth, ivec2(CSVoxelPosition.xy)).x);
            
            if (Depth != 0)
            {
                const vec2 LambdaPoint = (CSVoxelPosition.xy - g_FocalPoint) * g_InvFocalLength;
                const float Lambda = length(vec3(LambdaPoint, 1.0f));

                const float SDF = Depth - 1000.0f * length(VSVoxelPosition) / Lambda;

                const float DistanceFromCamera = length(CameraPosition - WSVoxelPosition);

                if (abs(DistanceFromCamera - Depth / 1000.0f) < 0.001f)
                {
                    imageStore(cs_Volume, VoxelCoords, uvec4(1, 1, 0, 0));
                }
                float TSDF = clamp(SDF / TRUNCATED_DISTANCE, -1.0f, 1.0f);
                
                imageStore(cs_Debug, VoxelCoords, vec4(SDF, TSDF, 0, 0));
            }
        }
    }
}

#endif // __INCLUDE_CS_KINECT_INTEGRATE_VOLUME_GLSL__