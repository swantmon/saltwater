
#ifndef __INCLUDE_CS_CONTOURS_GLSL__
#define __INCLUDE_CS_CONTOURS_GLSL__

#include "common_tracking.glsl"

// -----------------------------------------------------------------------------
// Defines
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Input from engine
// -----------------------------------------------------------------------------

layout (binding = 0, r16ui) readonly uniform uimage2D cs_DepthBuffer;
layout (binding = 1, rgba32f) writeonly uniform image2D cs_Contours;

// -------------------------------------------------------------------------------------
// Functions
// -------------------------------------------------------------------------------------

layout (local_size_x = TILE_SIZE2D, local_size_y = TILE_SIZE2D, local_size_z = 1) in;
void main()
{
    const int x = int(gl_GlobalInvocationID.x);
    const int y = int(gl_GlobalInvocationID.y);

    const ivec2 ImageSize = imageSize(cs_DepthBuffer);

    if (x > 0 && x < ImageSize.x - 1 && y > 0 && y < ImageSize.y - 1)
    {
        
    }
}

#endif // __INCLUDE_CS_CONTOURS_GLSL__