
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

#ifdef USE_SHUFFLE_INTRINSICS

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

#else

layout (local_size_x = REDUCTION_SHADER_COUNT, local_size_y = 1, local_size_z = 1) in;
void main()
{
    uint LeftIndex = gl_GlobalInvocationID.x;
    uint RightIndex = gl_GlobalInvocationID.x + g_SumCountPOT / 2;

    if (RightIndex < g_SumCount)
    {
        g_ICPData[LeftIndex][gl_GlobalInvocationID.y] += g_ICPData[RightIndex][gl_GlobalInvocationID.y];
    }

    int SumCountPOT = g_SumCountPOT / 4;

    barrier();

    while (SumCountPOT > 0)
    {
        if (gl_GlobalInvocationID.x < SumCountPOT)
        {
            g_ICPData[gl_GlobalInvocationID.x][gl_GlobalInvocationID.y] += g_ICPData[gl_GlobalInvocationID.x + SumCountPOT][gl_GlobalInvocationID.y];
        }

        SumCountPOT /= 2;

        barrier();
    }
}

#endif

#endif // __INCLUDE_CS_REDUCE_SUMMANDS_GLSL__