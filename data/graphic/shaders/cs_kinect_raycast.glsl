
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
    float xmin = ((Direction.x > 0.0 ? - VOLUME_SIZE / 2 : VOLUME_SIZE / 2) - Start.x) / Direction.x;
    float ymin = ((Direction.y > 0.0 ? - VOLUME_SIZE / 2 : VOLUME_SIZE / 2) - Start.y) / Direction.y;
    float zmin = ((Direction.z > 0.0 ? - VOLUME_SIZE / 2 : VOLUME_SIZE / 2) - Start.z) / Direction.z;

    return max(max(xmin, ymin), zmin);
}

float GetEndLength(vec3 Start, vec3 Direction)
{
    float xmax = ((Direction.x > 0.0 ? VOLUME_SIZE / 2 : - VOLUME_SIZE / 2) - Start.x) / Direction.x;
    float ymax = ((Direction.y > 0.0 ? VOLUME_SIZE / 2 : - VOLUME_SIZE / 2) - Start.y) / Direction.y;
    float zmax = ((Direction.z > 0.0 ? VOLUME_SIZE / 2 : - VOLUME_SIZE / 2) - Start.z) / Direction.z;

    return min(min(xmax, ymax), zmax);
}

uvec2 GetVoxel(vec3 Position)
{
    vec3 SamplePosition = Position;
    SamplePosition /= VOLUME_SIZE;
    SamplePosition += 0.5;
    SamplePosition *= VOLUME_RESOLUTION;

    return uvec2(imageLoad(cs_Volume, ivec3(SamplePosition)).xy);
}

layout (local_size_x = TILE_SIZE2D, local_size_y = TILE_SIZE2D, local_size_z = 1) in;
void main()
{
    ivec2 VertexMapSize = imageSize(cs_Vertex);

    const ivec2 VertexMapPosition = ivec2(gl_GlobalInvocationID.xy);
    
    vec3 VertexPosition = vec3(vec2(VertexMapPosition) / VertexMapSize.xy, 0.0);
    VertexPosition.xy -= vec2(0.5);
    VertexPosition.xy *= vec2(5.0);

    vec3 CameraPosition = vec3(0.0, 0.0, 10.0);
    vec3 RayDirection = normalize(VertexPosition - CameraPosition);

    RayDirection.x = RayDirection.x == 0.0 ? 1e-15 : RayDirection.x;
    RayDirection.y = RayDirection.y == 0.0 ? 1e-15 : RayDirection.y;
    RayDirection.z = RayDirection.z == 0.0 ? 1e-15 : RayDirection.z;

    float StartLength = GetStartLength(CameraPosition, RayDirection);
    float EndLength = GetEndLength(CameraPosition, RayDirection);

    float Step = VOXEL_SIZE;
    float Ray = StartLength;

    vec3 Vertex = vec3(0.0);

    while (Ray <= EndLength)
    {
        vec3 CurrentPosition = CameraPosition + Ray * RayDirection;

        uvec2 Voxel = GetVoxel(CurrentPosition);
        if (Voxel.x > 0)
        {
            Vertex = CurrentPosition;
            break;
        }

        Ray += Step;
    }

    imageStore(cs_Vertex, VertexMapPosition, vec4(Vertex, 1.0));
}

#endif // __INCLUDE_CS_KINECT_INTEGRATE_VOLUME_GLSL__