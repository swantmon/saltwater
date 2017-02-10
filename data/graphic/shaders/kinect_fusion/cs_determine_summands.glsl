
#ifndef __INCLUDE_CS_DETERMINE_SUMMANDS_GLSL__
#define __INCLUDE_CS_DETERMINE_SUMMANDS_GLSL__

#include "tracking_common.glsl"

// -----------------------------------------------------------------------------
// Constants
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Input from engine
// -----------------------------------------------------------------------------

layout(binding = 0, rgba32f) uniform image2D cs_VertexMap;
layout(binding = 1, rgba32f) uniform image2D cs_NormalMap;
layout(binding = 2, rgba32f) uniform image2D cs_RaycastVertexMap;
layout(binding = 3, rgba32f) uniform image2D cs_RaycastNormalMap;

layout(binding = 4, rgba32f) uniform image2D cs_Debug;

// -------------------------------------------------------------------------------------
// Functions
// -------------------------------------------------------------------------------------

layout (local_size_x = TILE_SIZE2D, local_size_y = TILE_SIZE2D, local_size_z = 1) in;
void main()
{
    const int x = int(gl_GlobalInvocationID.x);
    const int y = int(gl_GlobalInvocationID.y);

    const ivec2 ImageSize = imageSize(cs_VertexMap);
    const int PyramidLevel = int(log2(DEPTH_IMAGE_WIDTH / ImageSize.x));
    
    vec3 ReferenceVertex = imageLoad(cs_VertexMap, ivec2(x, y)).xyz;

    if (ReferenceVertex.x == 0.0f)
    {
        return;
    }

    vec3 Vertex = (g_InvPoseMatrix * vec4(ReferenceVertex, 1.0)).xyz;
    vec3 CameraPlane = mat3(g_Intrinisics[PyramidLevel].m_KMatrix) * Vertex;
    CameraPlane /= CameraPlane.z;

    if (CameraPlane.x < 0.0f || CameraPlane.x > ImageSize.x ||
        CameraPlane.y < 0.0f || CameraPlane.y > ImageSize.y)
    {
        return;
    }

    vec3 ReferenceNormal = imageLoad(cs_NormalMap, ivec2(x, y)).xyz;

    if (ReferenceNormal.x == 0.0f)
    {
        return;
    }

    vec3 RaycastVertex = imageLoad(cs_RaycastVertexMap, ivec2(CameraPlane.xy)).xyz;
    vec3 RaycastNormal = imageLoad(cs_RaycastNormalMap, ivec2(CameraPlane.xy)).xyz;
    
    const float Distance = distance(ReferenceVertex, RaycastVertex);
    const float Angle = dot(ReferenceNormal, RaycastNormal);

    if (Distance > EPSILON_DISTANCE || Angle < EPSILON_ANGLE)
    {
        return;
    }
    
    imageStore(cs_Debug, ivec2(x, y), vec4(Distance, Angle, 0.0f, 1.0f));
}

#endif // __INCLUDE_CS_DETERMINE_SUMMANDS_GLSL__