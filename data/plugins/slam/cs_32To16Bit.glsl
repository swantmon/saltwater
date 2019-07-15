#ifndef __INCLUDE_CS_32_TO_16_GLSL__
#define __INCLUDE_CS_32_TO_16_GLSL__

layout(binding = 0, r32ui) readonly uniform uimage2D cs_Depth32;
layout(binding = 1, r16ui) writeonly uniform uimage2D cs_Depth;

layout (local_size_x = TILE_SIZE_2D, local_size_y = TILE_SIZE_2D, local_size_z = 1) in;
void main()
{
    ivec2 Coords = ivec2(gl_GlobalInvocationID.xy);

    uint Depth = imageLoad(cs_Depth32, Coords).x;

    Depth = Depth == 0xFFFFFFFF ? 0 : Depth;

    imageStore(cs_Depth, Coords, uvec4(Depth));
}

#endif //__INCLUDE_CS_32_TO_16_GLSL__