#ifndef __INCLUDE_CS_SUM_FINAL_GLSL__
#define __INCLUDE_CS_SUM_FINAL_GLSL__

#include "../../plugins/slam/scalable/registration/common.glsl"

shared vec4 g_SharedData[REDUCTION_SHADER_COUNT];

void reduce()
{
    for (int i = REDUCTION_SHADER_COUNT; i >= 1; i /= 2)
    {
        if (gl_LocalInvocationIndex < i / 2)
        {
            g_SharedData[gl_LocalInvocationIndex] += g_SharedData[gl_LocalInvocationIndex + i / 2];
        }
        barrier();
    }
}

#ifndef USE_MULTIPLE_SUMMATION_PASSES

/////////////////////////////////////////////////////////////////////////////////////
// do everything on on step
/////////////////////////////////////////////////////////////////////////////////////

layout (local_size_x = REDUCTION_SHADER_COUNT, local_size_y = 1, local_size_z = 1) in;
void main()
{
    g_SharedData[gl_LocalInvocationIndex] = g_Sum[gl_LocalInvocationIndex];

    barrier();

    reduce();

    if (gl_LocalInvocationIndex == 0)
    {
        g_Sum[0] = g_SharedData[0];
    }
}

#else

/////////////////////////////////////////////////////////////////////////////////////
// loop first because we have more summands than threads
/////////////////////////////////////////////////////////////////////////////////////

layout (local_size_x = REDUCTION_SHADER_COUNT, local_size_y = 1, local_size_z = 1) in;
void main()
{
    const int LoopCount = int(float(REDUCTION_SHADER_COUNT) / WORKGROUP_SUMMANDS + 0.5f);

    vec4 Sum = vec4(0.0f);

    for (int i = 0; i < LoopCount; ++ i)
    {
        int Index = int(gl_LocalInvocationIndex) * LoopCount + i;
        if (Index < WORKGROUP_SUMMANDS)
        {
            Sum += g_Sum[Index];
        }
    }

    g_SharedData[gl_LocalInvocationIndex] = Sum;

    barrier();

    reduce();

    if (gl_LocalInvocationIndex == 0)
    {
        g_Sum[0] = g_SharedData[0];
    }
}

#endif

#endif //__INCLUDE_CS_SUM_FINAL_GLSL__