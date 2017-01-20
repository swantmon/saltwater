
#ifndef __INCLUDE_CS_KINECT_INTEGRATE_VOLUME_GLSL__
#define __INCLUDE_CS_KINECT_INTEGRATE_VOLUME_GLSL__

#include "tracking_common.glsl"

// -----------------------------------------------------------------------------
// Constants
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Input from engine
// -----------------------------------------------------------------------------

layout (binding = 0, rg16ui) readonly uniform uimage3D cs_Volume;
layout (binding = 1, rgba32f) writeonly uniform image2D cs_Vertex;

// -------------------------------------------------------------------------------------
// Functions
// -------------------------------------------------------------------------------------

layout (local_size_x = TILE_SIZE2D, local_size_y = TILE_SIZE2D, local_size_z = 1) in;
void main()
{
    ivec3 VolumeSize = imageSize(cs_Volume);

    const ivec2 VertexMapPosition = ivec2(gl_GlobalInvocationID.xy);

    vec3 CameraPosition = vec3(0.0, 10.0, 0.0);
    vec3 RayDirection = normalize(-CameraPosition);
    vec3 CurrentPosition = CameraPosition;

    bool Hit = false;

    while (!Hit)
    {
        ivec3 SamplePosition = ivec3(CurrentPosition * (VolumeSize / 2));

        ivec2 Voxel = ivec2(imageLoad(cs_Volume, SamplePosition).rg);

        if (Voxel.x > 0)
        {
            Hit = true;

            imageStore(cs_Vertex, VertexMapPosition, vec4(CurrentPosition, 1.0));
        }

        CurrentPosition += vec3(VOXEL_SIZE);

        if (true)
        {
            imageStore(cs_Vertex, VertexMapPosition, vec4(0.0, 0.0, 0.0, 1.0));
            break;
        }
    }
}

#endif // __INCLUDE_CS_KINECT_INTEGRATE_VOLUME_GLSL__