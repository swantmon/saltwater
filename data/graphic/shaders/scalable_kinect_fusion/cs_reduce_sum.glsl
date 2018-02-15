
#extension GL_NV_shader_thread_shuffle : enable
#extension GL_NV_shader_thread_group : enable

#ifndef __INCLUDE_CS_REDUCE_SUMMANDS_GLSL__
#define __INCLUDE_CS_REDUCE_SUMMANDS_GLSL__

#include "scalable_kinect_fusion/common_tracking.glsl"

layout(std140, binding = 2) uniform UBOSummationData
{
    int g_SumCount;
    int g_SumCountPOT;
};

// -----------------------------------------------------------------------------
// Functions
// -----------------------------------------------------------------------------

shared float g_SharedData[REDUCTION_SHADER_COUNT / 32];

layout (local_size_x = REDUCTION_SHADER_COUNT, local_size_y = 1, local_size_z = 1) in;
void main()
{
    const uint SummandIndex = gl_GlobalInvocationID.x;
    const uint ICPIndex = gl_GlobalInvocationID.y;

    float Data = g_ICPData[SummandIndex][ICPIndex];

    if(SummandIndex + REDUCTION_SHADER_COUNT < g_SumCount)
    {
        Data += g_ICPData[SummandIndex + REDUCTION_SHADER_COUNT][ICPIndex];
    }

    Data += shuffleDownNV(Data, 16, 32);
    Data += shuffleDownNV(Data,  8, 32);
    Data += shuffleDownNV(Data,  4, 32);
    Data += shuffleDownNV(Data,  2, 32);
    Data += shuffleDownNV(Data,  1, 32);

#if REDUCTION_SHADER_COUNT > 32
    if (gl_LocalInvocationIndex % 32 == 0)
    {
        g_SharedData[gl_LocalInvocationIndex / 32] = Data;
    }

    barrier();

    Data = g_SharedData[gl_LocalInvocationIndex];

    for (int i = REDUCTION_SHADER_COUNT / 32; i >= 1; i /= 2)
    {
        Data += shuffleDownNV(Data, i, 32);
    }
#endif

    if (gl_LocalInvocationIndex == 0)
    {
        g_ICPData[0][ICPIndex] = Data;
    }
}

#endif // __INCLUDE_CS_REDUCE_SUMMANDS_GLSL__