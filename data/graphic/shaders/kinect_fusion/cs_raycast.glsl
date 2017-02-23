
#ifndef __INCLUDE_CS_KINECT_INTEGRATE_VOLUME_GLSL__
#define __INCLUDE_CS_KINECT_INTEGRATE_VOLUME_GLSL__

#include "tracking_common.glsl"

// -----------------------------------------------------------------------------
// Constants
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Input from engine
// -----------------------------------------------------------------------------

layout (binding = 0) uniform isampler3D cs_Volume;
layout (binding = 1, rgba32f) writeonly uniform image2D cs_Vertex;
layout (binding = 2, rgba32f) writeonly uniform image2D cs_Normal;

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

ivec3 GetVoxelCoords(vec3 Position)
{
    return ivec3(Position / VOXEL_SIZE + 0.5f);
}

vec2 GetVoxel(ivec3 Coords)
{
    vec2 Voxel = texelFetch(cs_Volume, Coords, 0).xy;
    Voxel.x /= float(INT16_MAX);
    return Voxel;
}

float GetInterPolatedTSDF(vec3 Position)
{
    vec3 Coords = GetVoxelCoords(Position);
        
    return textureLod(cs_Volume, Coords / float(VOLUME_RESOLUTION), 0).x / float(INT16_MAX);
}

layout (local_size_x = TILE_SIZE2D, local_size_y = TILE_SIZE2D, local_size_z = 1) in;
void main()
{
    ivec2 VertexMapSize = ivec2(DEPTH_IMAGE_WIDTH, DEPTH_IMAGE_HEIGHT);

    const ivec2 VertexMapPosition = ivec2(gl_GlobalInvocationID.xy);

    const vec2 FocalPoint = g_Intrinisics[0].m_FocalPoint;
    const vec2 InvFocalLength = g_Intrinisics[0].m_InvFocalLength;

    vec3 VertexPixelPosition;
    VertexPixelPosition.xy = vec2(VertexMapPosition - FocalPoint) * InvFocalLength;
    VertexPixelPosition.z = 1.0f;

    const vec3 CameraPosition = g_PoseMatrix[3].xyz;

    vec3 RayDirection = normalize(VertexPixelPosition);

    RayDirection = mat3(g_PoseMatrix) * RayDirection;

    RayDirection.x = RayDirection.x == 0.0f ? 1e-15f : RayDirection.x;
    RayDirection.y = RayDirection.y == 0.0f ? 1e-15f : RayDirection.y;
    RayDirection.z = RayDirection.z == 0.0f ? 1e-15f : RayDirection.z;

    const float StartLength = GetStartLength(CameraPosition, RayDirection);
    const float EndLength = GetEndLength(CameraPosition, RayDirection);

    const float Step = VOXEL_SIZE;
    float RayLength = StartLength;

    vec2 Voxel = GetVoxel(GetVoxelCoords(CameraPosition + RayLength * RayDirection));

    float TSDF = Voxel.x;

    vec3 Vertex = vec3(0.0f);

    while (RayLength <= EndLength)
    {
        vec3 PreviousPosition = CameraPosition + RayLength * RayDirection;
        RayLength += Step;
        vec3 CurrentPosition = CameraPosition + RayLength * RayDirection;

        float PreviousTSDF = TSDF;
        
        ivec3 VoxelCoords = GetVoxelCoords(CurrentPosition);
        
        vec2 Voxel = GetVoxel(VoxelCoords);

        TSDF = Voxel.x;

        if (PreviousTSDF > 0.0f && TSDF < 0.0f)
        {
            float Ft = GetInterPolatedTSDF(PreviousPosition);
            float Ftdt = GetInterPolatedTSDF(CurrentPosition);
            float Ts = RayLength - Step * Ft / (Ftdt - Ft);

            Vertex = CameraPosition + RayDirection * Ts;

            break;
        }
    }

    imageStore(cs_Vertex, VertexMapPosition, vec4(Vertex, 1.0f));
}

#endif // __INCLUDE_CS_KINECT_INTEGRATE_VOLUME_GLSL__