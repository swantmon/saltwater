
#ifndef __INCLUDE_CS_INPAINT_DEPTH_GLSL__
#define __INCLUDE_CS_INPAINT_DEPTH_GLSL__

#include "common_tracking.glsl"

// -----------------------------------------------------------------------------
// Defines
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Input from engine
// -----------------------------------------------------------------------------

layout(binding = 0, r16ui) uniform uimage2D cs_DepthBuffer;
layout(binding = 1, r16ui) uniform uimage2D cs_InpaintedDepthBuffer;

// -------------------------------------------------------------------------------------
// Functions
// -------------------------------------------------------------------------------------

uint Search(ivec2 Pos, int _Direction)
{
    while (Pos.x > 0 && Pos.x < imageSize(cs_DepthBuffer).x)
    {
        Pos.x += _Direction;

        const uint Depth = uint(imageLoad(cs_DepthBuffer, Pos).x);

        if (Depth != 0)
        {
            return Depth;
        }        
    }

    return 0;
}

layout (local_size_x = TILE_SIZE2D, local_size_y = TILE_SIZE2D, local_size_z = 1) in;
void main()
{
    const int x = int(gl_GlobalInvocationID.x);
    const int y = int(gl_GlobalInvocationID.y);

    const uint Depth = uint(imageLoad(cs_DepthBuffer, ivec2(x, y)).x);

    if (Depth == 0)
    {
        const uint Left = Search(ivec2(x, y), -1);
        const uint Right = Search(ivec2(x, y), 1);

        const uint Max = max(Left, Right);

        imageStore(cs_InpaintedDepthBuffer, ivec2(x, y), uvec4(Max));
    }
    else
    {
        imageStore(cs_InpaintedDepthBuffer, ivec2(x, y), uvec4(Depth));
    }
}

#endif // __INCLUDE_CS_INPAINT_DEPTH_GLSL__