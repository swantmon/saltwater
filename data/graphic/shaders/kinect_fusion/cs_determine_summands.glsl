
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

layout(row_major, std140, binding = 2) uniform UBOInc
{
    mat4 g_IncPoseMatrix;
    mat4 g_InvIncPoseMatrix;
};

// -----------------------------------------------------------------------------

shared float g_SharedData[WORKGROUP_SIZE];

// -------------------------------------------------------------------------------------
// Functions
// -------------------------------------------------------------------------------------

bool findCorrespondence(out vec3 ReferenceVertex, out vec3 RaycastVertex, out vec3 RaycastNormal)
{
    const int x = int(gl_GlobalInvocationID.x);
    const int y = int(gl_GlobalInvocationID.y);

    const ivec2 ImageSize = imageSize(cs_VertexMap);
    const int PyramidLevel = int(log2(DEPTH_IMAGE_WIDTH / ImageSize.x));

    ReferenceVertex = imageLoad(cs_VertexMap, ivec2(x, y)).xyz;

    if (ReferenceVertex.x == 0.0f)
    {
        return false;
    }

    ReferenceVertex = (g_IncPoseMatrix * vec4(ReferenceVertex, 1.0)).xyz;

    vec3 Vertex = (g_InvPoseMatrix * vec4(ReferenceVertex, 1.0)).xyz;
    
    vec3 CameraPlane = mat3(g_Intrinisics[PyramidLevel].m_KMatrix) * Vertex;
    CameraPlane /= CameraPlane.z;

    if (CameraPlane.x < 0.0f || CameraPlane.x > ImageSize.x ||
        CameraPlane.y < 0.0f || CameraPlane.y > ImageSize.y)
    {
        return false;
    }

    vec3 ReferenceNormal = imageLoad(cs_NormalMap, ivec2(x, y)).xyz;

    if (ReferenceNormal.x == 0.0f)
    {
        return false;
    }

    ReferenceNormal = (g_IncPoseMatrix * vec4(ReferenceNormal, 0.0)).xyz;

    RaycastVertex = (vec4(imageLoad(cs_RaycastVertexMap, ivec2(CameraPlane.xy)).xyz, 1.0)).xyz;
    RaycastNormal = (vec4(imageLoad(cs_RaycastNormalMap, ivec2(CameraPlane.xy)).xyz, 0.0)).xyz;

    const float Distance = distance(ReferenceVertex, RaycastVertex);
    const float Angle = dot(ReferenceNormal, RaycastNormal);

    if (Distance > EPSILON_DISTANCE || Angle < EPSILON_ANGLE)
    {
        return false;
    }

    RaycastVertex = (vec4(RaycastVertex, 1.0)).xyz;
    RaycastNormal = (vec4(RaycastNormal, 0.0)).xyz;
    
    return true;
}

void reduce()
{
    int SumCount = WORKGROUP_SIZE / 2;

    while (SumCount > 0)
    {
        if (gl_LocalInvocationIndex < SumCount)
        {
            g_SharedData[gl_LocalInvocationIndex] += g_SharedData[gl_LocalInvocationIndex + SumCount];
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

    vec3 ReferenceVertex;
    vec3 RaycastVertex;
    vec3 RaycastNormal;

    bool CorresponenceFound = findCorrespondence(ReferenceVertex, RaycastVertex, RaycastNormal);

    float Row[7];

    if (CorresponenceFound)
    {
        vec3 Cross = cross(ReferenceVertex, RaycastNormal);

        Row[0] = Cross.x;
        Row[1] = Cross.y;
        Row[2] = Cross.z;
        Row[3] = RaycastNormal.x;
        Row[4] = RaycastNormal.y;
        Row[5] = RaycastNormal.z;
        Row[6] = dot(RaycastNormal, RaycastVertex - ReferenceVertex);
    }
    else
    {
        Row[0] = Row[1] = Row[2] = Row[3] = Row[4] = Row[5] = Row[6] = 0.0f;
    }
    
    const uint ICPSummandIndex = gl_WorkGroupID.x + gl_WorkGroupID.y * gl_NumWorkGroups.x;
    int ICPValueIndex = 0;
    
    for (int i = 0; i < 6; ++ i)
    {
        for (int j = i; j < 7; ++ j)
        {
            barrier();
            
            g_SharedData[gl_LocalInvocationIndex] = Row[i] * Row[j];

            barrier();

            reduce();
            
            if (gl_LocalInvocationIndex == 0)
            {
                g_ICPData[ICPSummandIndex][ICPValueIndex++] = g_SharedData[0];
            }
        }
    }
}

#endif // __INCLUDE_CS_DETERMINE_SUMMANDS_GLSL__