#ifndef __INCLUDE_CS_SUM_TILES_GLSL__
#define __INCLUDE_CS_SUM_TILES_GLSL__

#include "../../plugins/slam/scalable/registration/common.glsl"

layout (local_size_x = TILE_SIZE2D, local_size_y = TILE_SIZE2D, local_size_z = 1) in;
void main()
{
    ivec2 Coords = ivec2(gl_GlobalInvocationID.xy);
}

#endif //__INCLUDE_CS_SUM_TILES_GLSL__