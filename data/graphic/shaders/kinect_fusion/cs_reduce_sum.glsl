
#ifndef __INCLUDE_CS_REDUCE_SUMMANDS_GLSL__
#define __INCLUDE_CS_REDUCE_SUMMANDS_GLSL__

#include "tracking_common.glsl"

layout(row_major, std140, binding = 2) uniform UBOSummationData
{
    int g_SumCount;
    int g_SumCountPOT;
};

// -----------------------------------------------------------------------------
// Functions
// -----------------------------------------------------------------------------

layout (local_size_x = REDUCTION_SHADER_COUNT, local_size_y = 1, local_size_z = 1) in;
void main()
{
    uint LeftIndex = gl_GlobalInvocationID.x;
    uint RightIndex = gl_GlobalInvocationID.x + g_SumCountPOT;

    if (RightIndex < g_SumCount * 2)
    {
        g_ICPData[LeftIndex][gl_GlobalInvocationID.y] += g_ICPData[RightIndex][gl_GlobalInvocationID.y];
    }

    int SumCountPOT = g_SumCountPOT / 2;

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

#endif // __INCLUDE_CS_REDUCE_SUMMANDS_GLSL__