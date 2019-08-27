#ifndef __INCLUDE_CS_SOBEL_GLSL__
#define __INCLUDE_CS_SOBEL_GLSL__

#include "../../plugins/slam/scalable/registration/common.glsl"

float ComputeConvolution(mat3 A, mat3 Kernel)
{
    float Gradient = 0.0f;
    for (int i = 0; i < 3; ++ i)
    {
        for (int j = 0; j < 3; ++ j)
        {
            Gradient += A[i][j] * Kernel[i][j];
        }
    }
    return Gradient;
}

layout (local_size_x = TILE_SIZE2D, local_size_y = TILE_SIZE2D, local_size_z = 1) in;
void main()
{
    ivec2 Coords = ivec2(gl_GlobalInvocationID.xy);

    mat3 KernelX = mat3(-1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
    mat3 KernelY = mat3(-1.0f, 0.0f, 1.0f, -1.0f, 0.0f, 1.0f, -1.0f, 0.0f, 1.0f);

    mat3 A; 
    for (int i = 0; i < 3; ++ i)
    {
        for (int j = 0; j < 3; ++ j)
        {
            A[i][j] = imageLoad(MovingImage, Coords + ivec2(i, j) - 1).x;
        }
    }
    
    vec2 G = vec2(ComputeConvolution(A, KernelX), ComputeConvolution(A, KernelY));
    imageStore(GradientImage, Coords, vec4(G, length(G), atan(G.y, G.x)));
}

#endif //__INCLUDE_CS_SOBEL_GLSL__