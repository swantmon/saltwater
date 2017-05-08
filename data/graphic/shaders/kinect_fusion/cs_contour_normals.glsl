
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

layout (local_size_x = TILE_SIZE2D, local_size_y = TILE_SIZE2D, local_size_z = 1) in;
void main()
{
    const int x = int(gl_GlobalInvocationID.x);
    const int y = int(gl_GlobalInvocationID.y);

    imageStore(cs_NormalBuffer, ivec2(x, y), vec4(0.5, 0.5, 0.5, 1.0));
}

#endif // __INCLUDE_CS_CONTOURS_NORMAL_GLSL__