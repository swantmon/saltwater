
#extension GL_NV_shader_thread_shuffle : enable
#extension GL_NV_shader_thread_group : enable

#ifndef __INCLUDE_CS_DETERMINE_SUMMANDS_GLSL__
#define __INCLUDE_CS_DETERMINE_SUMMANDS_GLSL__

#include "kinect_fusion/common_tracking.glsl"

#define WORKGROUP_SIZE (TILE_SIZE2D) * (TILE_SIZE2D)

// -------------------------------------------------------------------------------------
// Input from engine
// -------------------------------------------------------------------------------------

layout(binding = 0, MAP_TEXTURE_FORMAT) uniform image2D cs_VertexMap;
layout(binding = 1, MAP_TEXTURE_FORMAT) uniform image2D cs_NormalMap;
layout(binding = 2, MAP_TEXTURE_FORMAT) uniform image2D cs_RaycastVertexMap;
layout(binding = 3, MAP_TEXTURE_FORMAT) uniform image2D cs_RaycastNormalMap;

layout(std140, binding = 2) uniform UBOInc
{
    mat4 g_IncPoseMatrix;
    mat4 g_InvIncPoseMatrix;
    int g_PyramidLevel;
};

// -------------------------------------------------------------------------------------

#ifdef USE_SHUFFLE_INTRINSICS

shared float g_SharedData[8];

void reduce(float _Input)
{
    float Data = _Input;

    Data += shuffleDownNV(Data, 16, 32);
    Data += shuffleDownNV(Data,  8, 32);
    Data += shuffleDownNV(Data,  4, 32);
    Data += shuffleDownNV(Data,  2, 32);
    Data += shuffleDownNV(Data,  1, 32);

    if (gl_LocalInvocationIndex % 32 == 0)
    {
        g_SharedData[gl_LocalInvocationIndex / 32] = Data;
    }

    barrier();

    if (gl_LocalInvocationIndex < 8)
    {
        Data = g_SharedData[gl_LocalInvocationIndex];

        Data += shuffleDownNV(Data,  4, 32);
        Data += shuffleDownNV(Data,  2, 32);
        Data += shuffleDownNV(Data,  1, 32);

        if (gl_LocalInvocationIndex == 0)
        {
            g_SharedData[0] == Data; 
        }
    }

    barrier();
}

#else

shared float g_SharedData[WORKGROUP_SIZE];

void reduce()
{
	for (int i = WORKGROUP_SIZE; i >= 1; i /= 2)
	{
		if (gl_LocalInvocationIndex < i / 2)
		{
			g_SharedData[gl_LocalInvocationIndex] += g_SharedData[gl_LocalInvocationIndex + i / 2];
		}
		barrier();
	}
}

#endif

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

    vec3 CameraPlane = mat3(g_Intrinsics[g_PyramidLevel].m_KMatrix) * (g_InvPoseMatrix * vec4(ReferenceVertex, 1.0f)).xyz;
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

    if (isnan(Row[0]) || isnan(Row[1]) || isnan(Row[2]) || isnan(Row[3]) || isnan(Row[4]) || isnan(Row[5]) || isnan(Row[6]))
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
            
            #ifdef USE_SHUFFLE_INTRINSICS

            reduce(Row[i] * Row[j]);
            
            if (gl_LocalInvocationIndex == 0)
            {
                g_ICPData[ICPSummandIndex][ICPValueIndex++] = g_SharedData[0];
            }
            #else

            g_SharedData[gl_LocalInvocationIndex] = Row[i] * Row[j];

            barrier();

            reduce();
            
            if (gl_LocalInvocationIndex == 0)
            {
                g_ICPData[ICPSummandIndex][ICPValueIndex++] = g_SharedData[0];
            }
            
            #endif
        }
    }
}

#endif // __INCLUDE_CS_DETERMINE_SUMMANDS_GLSL__