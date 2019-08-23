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

#endif //__INCLUDE_CS_SUM_FINAL_GLSL__