#ifndef __INCLUDE_CS_SUM_TILES_GLSL__
#define __INCLUDE_CS_SUM_TILES_GLSL__

#include "../../plugins/slam/scalable/registration/common.glsl"

shared vec4 g_SharedData[TILE_SIZE2D * TILE_SIZE2D];

void reduce()
{
    for (int i = TILE_SIZE2D * TILE_SIZE2D; i >= 1; i /= 2)
    {
        if (gl_LocalInvocationIndex < i / 2)
        {
            g_SharedData[gl_LocalInvocationIndex] += g_SharedData[gl_LocalInvocationIndex + i / 2];
        }
        barrier();
    }
}

layout (local_size_x = TILE_SIZE2D, local_size_y = TILE_SIZE2D, local_size_z = 1) in;
void main()
{
    ivec2 Coords = ivec2(gl_GlobalInvocationID.xy);

    g_SharedData[gl_LocalInvocationIndex] = imageLoad(cs_Gradient, Coords);
    barrier();

    reduce();

    if (gl_LocalInvocationIndex == 0)
    {
        g_Sum[gl_WorkGroupID.x * TILE_SIZE2D + gl_WorkGroupID.y] = g_SharedData[0];
    }
}

#endif //__INCLUDE_CS_SUM_TILES_GLSL__