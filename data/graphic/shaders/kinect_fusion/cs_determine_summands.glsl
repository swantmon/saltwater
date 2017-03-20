
#ifndef __INCLUDE_CS_DETERMINE_SUMMANDS_GLSL__
#define __INCLUDE_CS_DETERMINE_SUMMANDS_GLSL__

#include "common_tracking.glsl"

#define WORKGROUP_SIZE (TILE_SIZE2D) * (TILE_SIZE2D)

// -------------------------------------------------------------------------------------
// Input from engine
// -------------------------------------------------------------------------------------

layout(binding = 0, rgba32f) uniform image2D cs_VertexMap;
layout(binding = 1, rgba32f) uniform image2D cs_NormalMap;
layout(binding = 2, rgba32f) uniform image2D cs_RaycastVertexMap;
layout(binding = 3, rgba32f) uniform image2D cs_RaycastNormalMap;
layout(binding = 4, rgba32f) uniform image2D cs_Debug;

layout(row_major, std140, binding = 2) uniform UBOInc
{
    mat4 g_IncPoseMatrix;
    mat4 g_InvIncPoseMatrix;
    int g_PyramidLevel;
};

// -------------------------------------------------------------------------------------

shared float g_SharedData[WORKGROUP_SIZE];

// -------------------------------------------------------------------------------------
// Functions
// -------------------------------------------------------------------------------------

void reduce()
{
    /*if (WORKGROUP_SIZE >= 512)
    {
        if (gl_LocalInvocationIndex < 256)
        {
            g_SharedData[gl_LocalInvocationIndex] += g_SharedData[gl_LocalInvocationIndex + 256];
        }
        barrier();
    }*/
    if (WORKGROUP_SIZE >= 256)
    {
        if (gl_LocalInvocationIndex < 128)
        {
            g_SharedData[gl_LocalInvocationIndex] += g_SharedData[gl_LocalInvocationIndex + 128];
        }
        barrier();
    }
    if (WORKGROUP_SIZE >= 128)
    {
        if (gl_LocalInvocationIndex < 64)
        {
            g_SharedData[gl_LocalInvocationIndex] += g_SharedData[gl_LocalInvocationIndex + 64];
        }
        barrier();
    }

    if (gl_LocalInvocationIndex < 32)
    {
        g_SharedData[gl_LocalInvocationIndex] += g_SharedData[gl_LocalInvocationIndex + 32];
        g_SharedData[gl_LocalInvocationIndex] += g_SharedData[gl_LocalInvocationIndex + 16];
        g_SharedData[gl_LocalInvocationIndex] += g_SharedData[gl_LocalInvocationIndex +  8];
        g_SharedData[gl_LocalInvocationIndex] += g_SharedData[gl_LocalInvocationIndex +  4];
        g_SharedData[gl_LocalInvocationIndex] += g_SharedData[gl_LocalInvocationIndex +  2];
        g_SharedData[gl_LocalInvocationIndex] += g_SharedData[gl_LocalInvocationIndex +  1];
    }
}

bool findCorrespondence(out vec3 ReferenceVertex, out vec3 RaycastVertex, out vec3 RaycastNormal)
{
    const int x = int(gl_GlobalInvocationID.x);
    const int y = int(gl_GlobalInvocationID.y);

    const ivec2 ImageSize = imageSize(cs_VertexMap);

    vec3 Vertex = imageLoad(cs_VertexMap, ivec2(x, y)).xyz;

    if (Vertex.x == 0.0f)
    {
        return false;
    }
    
    ReferenceVertex = (g_IncPoseMatrix * vec4(Vertex, 1.0)).xyz;

    vec3 CameraPlane = mat3(g_Intrinisics[g_PyramidLevel].m_KMatrix) * (g_InvPoseMatrix * vec4(ReferenceVertex, 1.0f)).xyz;
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

    ReferenceNormal = mat3(g_IncPoseMatrix) * ReferenceNormal;

    RaycastVertex = imageLoad(cs_RaycastVertexMap, ivec2(CameraPlane.xy)).xyz;
    RaycastNormal = imageLoad(cs_RaycastNormalMap, ivec2(CameraPlane.xy)).xyz;

    if (RaycastVertex.x == 0.0f || RaycastNormal.x == 0.0f)
    {
        return false;
    }
    
    const float Distance = distance(ReferenceVertex, RaycastVertex);
    const float Angle = dot(ReferenceNormal, RaycastNormal);

    if (Distance > EPSILON_DISTANCE || Angle < EPSILON_ANGLE)
    {
        return false;
    }
    
    return true;
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
    
    //imageStore(cs_Debug, ivec2(x, y), CorresponenceFound ? vec4(1.0f) : vec4(0.0f));

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