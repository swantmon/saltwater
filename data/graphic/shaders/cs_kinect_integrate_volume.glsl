
#ifndef __INCLUDE_CS_KINECT_INTEGRATE_VOLUME_GLSL__
#define __INCLUDE_CS_KINECT_INTEGRATE_VOLUME_GLSL__

#include "tracking_common.glsl"

// -----------------------------------------------------------------------------
// Constants
// -----------------------------------------------------------------------------

#define MAX_WEIGHT 100.0

// -----------------------------------------------------------------------------
// Input from engine
// -----------------------------------------------------------------------------

layout(binding = 0, rg16ui) uniform uimage3D cs_Volume;
layout(binding = 1, r16ui) readonly uniform uimage2D cs_Depth;
layout(binding = 2, rgba32f) writeonly uniform image3D cs_Debug;

// -------------------------------------------------------------------------------------
// Functions
// -------------------------------------------------------------------------------------

layout (local_size_x = TILE_SIZE2D, local_size_y = TILE_SIZE2D, local_size_z = 1) in;
void main()
{
	ivec2 ImageSize = ivec2(DEPTH_IMAGE_WIDTH, DEPTH_IMAGE_HEIGHT);
	
    const int x = int(gl_GlobalInvocationID.x);
    const int y = int(gl_GlobalInvocationID.y);

    vec3 vg;
    vg.x = (x + 0.5) * VOXEL_SIZE - g_PoseMatrix[3].x;
    vg.y = (y + 0.5) * VOXEL_SIZE - g_PoseMatrix[3].y;
    vg.z = (0 + 0.5) * VOXEL_SIZE - g_PoseMatrix[3].z;

    uvec3 pos = uvec3(x, y, 0);
    const int step = 1;

    for (int z = 0; z < VOLUME_RESOLUTION; ++ z, pos.z += step, vg.z += VOXEL_SIZE)
    {
        imageStore(cs_Volume, ivec3(x, y, z), uvec4(0)); // clear for debugging

        const vec3 v = mat3(g_InvPoseRotationMatrix) * vg;

        const float zFactor = 1.0 / v.z;

        const ivec2 cameraPlane = ivec2(v.xy * g_FocalLength * vec2(zFactor) + g_FocalPoint);

        if (v.z > 0.0 && cameraPlane.x >= 0 && cameraPlane.x < DEPTH_IMAGE_WIDTH && cameraPlane.y >= 0 && cameraPlane.y < DEPTH_IMAGE_HEIGHT)
        {
            const int depth = int(imageLoad(cs_Depth, cameraPlane).x);

            if (depth != 0)
            {
                const float lx = (cameraPlane.x - g_FocalPoint.x) * g_InvFocalLength.x;
                const float ly = (cameraPlane.y - g_FocalPoint.y) * g_InvFocalLength.y;
                const float lamdaInverse = 1.0 / sqrt(lx * lx + ly * ly + 1.0);

                const float sdf = float(depth) - 1000.0f * lamdaInverse * length(vg);
                //const float sdf = float(depth) - lamdaInverse * length(vg);

                if (sdf >= - TRUNCATED_DISTANCE)
                {
                    const float tsdf = min(1.0, sdf * TRUNCATED_DISTANCE_INVERSE);

                    const uvec2 volumeValue = uvec2(imageLoad(cs_Volume, ivec3(pos)).xy);

                    const float tsdfOld = float(volumeValue.x) / UINT16_MAX;
                    const int weightOld = int(volumeValue.y);

                    const float tsdfNew = float(weightOld * tsdfOld + tsdf) / float(weightOld + 1);
                    const int weightNew = int(min(weightOld + 1.0, MAX_WEIGHT));

                    imageStore(cs_Volume, ivec3(pos), uvec4(tsdfNew * UINT16_MAX, weightNew, 0, 0));
                }
            }
            //imageStore(cs_Debug, ivec3(pos), depth == 0 ? vec4(0) : vec4(1));
        }
        imageStore(cs_Debug, ivec3(x, y, z), vec4(cameraPlane, 0, 0));
    }
}

#endif // __INCLUDE_CS_KINECT_INTEGRATE_VOLUME_GLSL__