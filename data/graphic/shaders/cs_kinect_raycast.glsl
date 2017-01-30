
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

float GetStartLength(vec3 Start, vec3 Direction)
{
    float xmin = ((Direction.x > 0.0f ? 0.0f : VOLUME_SIZE) - Start.x) / Direction.x;
    float ymin = ((Direction.y > 0.0f ? 0.0f : VOLUME_SIZE) - Start.y) / Direction.y;
    float zmin = ((Direction.z > 0.0f ? 0.0f : VOLUME_SIZE) - Start.z) / Direction.z;

    return max(max(xmin, ymin), zmin);
}

float GetEndLength(vec3 Start, vec3 Direction)
{
    float xmax = ((Direction.x > 0.0f ? VOLUME_SIZE : 0.0f) - Start.x) / Direction.x;
    float ymax = ((Direction.y > 0.0f ? VOLUME_SIZE : 0.0f) - Start.y) / Direction.y;
    float zmax = ((Direction.z > 0.0f ? VOLUME_SIZE : 0.0f) - Start.z) / Direction.z;

    return min(min(xmax, ymax), zmax);
}

uvec2 GetVoxel(vec3 Position)
{
    return uvec2(imageLoad(cs_Volume, ivec3(Position / VOXEL_SIZE + 0.5f)).xy);
}

layout (local_size_x = TILE_SIZE2D, local_size_y = TILE_SIZE2D, local_size_z = 1) in;
void main()
{
    ivec2 VertexMapSize = ivec2(DEPTH_IMAGE_WIDTH, DEPTH_IMAGE_HEIGHT);

    const ivec2 VertexMapPosition = ivec2(gl_GlobalInvocationID.xy);
    
    vec3 VertexPixelPosition;
    VertexPixelPosition.xy = vec2(VertexMapPosition - g_FocalPoint) * g_InvFocalLength;
    VertexPixelPosition.z = 1.0f;

    const vec3 CameraPosition = g_PoseMatrix[3].xyz;

    vec3 RayDirection = normalize(VertexPixelPosition);

    RayDirection.x = RayDirection.x == 0.0f ? 1e-15f : RayDirection.x;
    RayDirection.y = RayDirection.y == 0.0f ? 1e-15f : RayDirection.y;
    RayDirection.z = RayDirection.z == 0.0f ? 1e-15f : RayDirection.z;

    const float StartLength = GetStartLength(CameraPosition, RayDirection);
    const float EndLength = GetEndLength(CameraPosition, RayDirection);

    const float Step = VOXEL_SIZE;
    float RayLength = StartLength;

    vec3 Vertex = vec3(0.0f);

    while (RayLength <= EndLength)
    {
        vec3 CurrentPosition = CameraPosition + RayLength * RayDirection;

        uvec2 Voxel = GetVoxel(CurrentPosition);
        if (Voxel.x > 0)
        {
            Vertex = CurrentPosition;
            break;
        }

        RayLength += Step;
    }

    imageStore(cs_Vertex, VertexMapPosition, vec4(Vertex, 1.0f));
}

#endif // __INCLUDE_CS_KINECT_INTEGRATE_VOLUME_GLSL__