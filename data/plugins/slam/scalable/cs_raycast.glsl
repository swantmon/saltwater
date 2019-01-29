
#ifndef __INCLUDE_CS_KINECT_INTEGRATE_VOLUME_GLSL__
#define __INCLUDE_CS_KINECT_INTEGRATE_VOLUME_GLSL__

#include "../../plugins/slam/common_tracking.glsl"
#include "../../plugins/slam/scalable/common_raycast.glsl"

// -----------------------------------------------------------------------------
// Constants
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Input from engine
// -----------------------------------------------------------------------------

layout (binding = 1, MAP_TEXTURE_FORMAT) writeonly uniform image2D cs_Vertex;
layout (binding = 2, MAP_TEXTURE_FORMAT) writeonly uniform image2D cs_Normal;

// -------------------------------------------------------------------------------------
// Functions
// -------------------------------------------------------------------------------------

layout (local_size_x = TILE_SIZE2D, local_size_y = TILE_SIZE2D, local_size_z = 1) in;
void main()
{
    ivec2 VertexMapSize = ivec2(DEPTH_IMAGE_WIDTH, DEPTH_IMAGE_HEIGHT);

    const ivec2 VertexMapPosition = ivec2(gl_GlobalInvocationID.xy);

    const vec2 FocalPoint = g_Intrinsics[0].m_FocalPoint;
    const vec2 InvFocalLength = g_Intrinsics[0].m_InvFocalLength;

    vec3 VertexPixelPosition;
    VertexPixelPosition.xy = vec2(VertexMapPosition - FocalPoint) * InvFocalLength;
    VertexPixelPosition.z = 1.0f;

    const vec3 CameraPosition = g_PoseMatrix[3].xyz;

    vec3 RayDirection = normalize(VertexPixelPosition);

    RayDirection = mat3(g_PoseMatrix) * RayDirection;

    RayDirection.x = RayDirection.x == 0.0f ? 1e-15f : RayDirection.x;
    RayDirection.y = RayDirection.y == 0.0f ? 1e-15f : RayDirection.y;
    RayDirection.z = RayDirection.z == 0.0f ? 1e-15f : RayDirection.z;
    
    vec3 Vertex = GetPosition(CameraPosition, RayDirection);
    imageStore(cs_Vertex, VertexMapPosition, vec4(Vertex, 1.0f));

#ifdef NORMAL_MAP_FROM_TSDF
    vec3 Normal = GetNormal(Vertex);
    imageStore(cs_Normal, VertexMapPosition, vec4(Normal, 1.0f));
#endif
}

#endif // __INCLUDE_CS_KINECT_INTEGRATE_VOLUME_GLSL__