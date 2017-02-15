
#ifndef __INCLUDE_CS_DETERMINE_SUMMANDS_GLSL__
#define __INCLUDE_CS_DETERMINE_SUMMANDS_GLSL__

#include "tracking_common.glsl"

#define WORKGROUP_SIZE (TILE_SIZE2D) * (TILE_SIZE2D)

// -----------------------------------------------------------------------------
// Input from engine
// -----------------------------------------------------------------------------

layout(binding = 0, rgba32f) uniform image2D cs_VertexMap;
layout(binding = 1, rgba32f) uniform image2D cs_NormalMap;
layout(binding = 2, rgba32f) uniform image2D cs_RaycastVertexMap;
layout(binding = 3, rgba32f) uniform image2D cs_RaycastNormalMap;

layout(binding = 4, rgba32f) uniform image2D cs_Debug;

// -----------------------------------------------------------------------------

shared float SharedData[WORKGROUP_SIZE];

// -------------------------------------------------------------------------------------
// Functions
// -------------------------------------------------------------------------------------

void reduce()
{
    int SumCount = WORKGROUP_SIZE / 2;

    while (SumCount > 0)
    {
        if (gl_LocalInvocationIndex < SumCount)
        {
            const float Sum = SharedData[gl_LocalInvocationIndex] + SharedData[gl_LocalInvocationIndex + SumCount];
            SharedData[gl_LocalInvocationIndex] = Sum;
        }

        SumCount /= 2;

        barrier();
    }
}

layout (local_size_x = TILE_SIZE2D, local_size_y = TILE_SIZE2D, local_size_z = 1) in;
void main()
{
    const int x = int(gl_GlobalInvocationID.x);
    const int y = int(gl_GlobalInvocationID.y);

    const ivec2 ImageSize = imageSize(cs_VertexMap);
    const int PyramidLevel = int(log2(DEPTH_IMAGE_WIDTH / ImageSize.x));
    
    bool CorresponenceFound = true;

    vec3 ReferenceVertex = imageLoad(cs_VertexMap, ivec2(x, y)).xyz;

    if (ReferenceVertex.x == 0.0f)
    {
        CorresponenceFound = false;
    }

    vec3 Vertex = (g_InvPoseMatrix * vec4(ReferenceVertex, 1.0)).xyz;
    vec3 CameraPlane = mat3(g_Intrinisics[PyramidLevel].m_KMatrix) * Vertex;
    CameraPlane /= CameraPlane.z;

    if (CameraPlane.x < 0.0f || CameraPlane.x > ImageSize.x ||
        CameraPlane.y < 0.0f || CameraPlane.y > ImageSize.y)
    {
        CorresponenceFound = false;
    }

    vec3 ReferenceNormal = imageLoad(cs_NormalMap, ivec2(x, y)).xyz;

    if (ReferenceNormal.x == 0.0f)
    {
        CorresponenceFound = false;
    }

    vec3 RaycastVertex = imageLoad(cs_RaycastVertexMap, ivec2(CameraPlane.xy)).xyz;
    vec3 RaycastNormal = imageLoad(cs_RaycastNormalMap, ivec2(CameraPlane.xy)).xyz;
    
    const float Distance = distance(ReferenceVertex, RaycastVertex);
    const float Angle = dot(ReferenceNormal, RaycastNormal);

    if (Distance > EPSILON_DISTANCE || Angle < EPSILON_ANGLE)
    {
        CorresponenceFound = false;
    }

    float Row[7];

    vec3 Cross = cross(Vertex, RaycastNormal);

    if (CorresponenceFound)
    {
        Row[0] = Cross.x;
        Row[1] = Cross.y;
        Row[2] = Cross.z;
        Row[3] = RaycastNormal.x;
        Row[4] = RaycastNormal.y;
        Row[5] = RaycastNormal.z;
        Row[6] = dot(RaycastNormal, RaycastVertex - Vertex);
    }
    else
    {
        Row[0] = Row[1] = Row[2] = Row[3] = Row[4] = Row[5] = Row[6] = 0.0f;
    }
    
    for (int i = 0; i < 6; ++ i)
    {
        for (int j = i; j < 7; ++ j)
        {
            barrier();
            SharedData[gl_LocalInvocationIndex] = Row[i] * Row[j];
            barrier();

            reduce();

            if (gl_LocalInvocationIndex == 0)
            {
                SharedData[0];
            }
        }
    }

    imageStore(cs_Debug, ivec2(x, y), vec4(Cross, SharedData[0]));
}

#endif // __INCLUDE_CS_DETERMINE_SUMMANDS_GLSL__