
#ifndef __INCLUDE_CS_PICKING_GLSL__
#define __INCLUDE_CS_PICKING_GLSL__

#include "slam/common_tracking.glsl"
#include "slam/scalable_kinect_fusion/common_raycast.glsl"

// -----------------------------------------------------------------------------
// Constants
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Input from engine
// -----------------------------------------------------------------------------

// -------------------------------------------------------------------------------------
// Functions
// -------------------------------------------------------------------------------------

layout (local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
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
}

#endif // __INCLUDE_CS_PICKING_GLSL__