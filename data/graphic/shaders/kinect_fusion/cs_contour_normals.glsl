
#ifndef __INCLUDE_CS_CONTOUR_NORMALS_GLSL__
#define __INCLUDE_CS_CONTOUR_NORMALS_GLSL__

#include "common_tracking.glsl"

// -----------------------------------------------------------------------------
// Defines
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Input from engine
// -----------------------------------------------------------------------------

layout(binding = 0, r16ui) uniform uimage2D cs_DepthBuffer;
layout(binding = 1, MAP_TEXTURE_FORMAT) uniform image2D cs_NormalBuffer;

// -------------------------------------------------------------------------------------
// Functions
// -------------------------------------------------------------------------------------

const int g_KernelSize = 5;

float g_SobelKernel[g_KernelSize * g_KernelSize] = {
    -0.25f, -0.20f, 0.0f, 0.20f, 0.25f,
    -0.20f, -0.50f, 0.0f, 0.50f, 0.20f,
    -0.50f, -1.00f, 0.0f, 1.00f, 0.50f,
    -0.20f, -0.50f, 0.0f, 0.50f, 0.20f,
    -0.25f, -0.20f, 0.0f, 0.20f, 0.25f,
};

vec2 ComputeGradient(ivec2 Position)
{
    vec2 Result = vec2(0.0f);

    for (int i = 0; i < g_KernelSize; ++i)
    {
        for (int j = 0; j < g_KernelSize; ++j)
        {
            float Sample = float(imageLoad(cs_DepthBuffer, Position + ivec2(i, j)));

            Result.x += Sample * g_SobelKernel[i * g_KernelSize + j];
            Result.y += Sample * g_SobelKernel[j * g_KernelSize + i];
        }
    }

    return Result;
}

layout (local_size_x = TILE_SIZE2D, local_size_y = TILE_SIZE2D, local_size_z = 1) in;
void main()
{
    const int x = int(gl_GlobalInvocationID.x);
    const int y = int(gl_GlobalInvocationID.y);
    
    imageStore(cs_NormalBuffer, ivec2(x, y), vec4(ComputeGradient(ivec2(x, y)), 0.0, 1.0));
}

#endif // __INCLUDE_CS_CONTOURS_NORMAL_GLSL__