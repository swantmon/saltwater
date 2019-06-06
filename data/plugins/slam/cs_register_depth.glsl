#ifndef __INCLUDE_CS_REGISTER_DEPTH_GLSL__
#define __INCLUDE_CS_REGISTER_DEPTH_GLSL__

layout(binding = 0, r16ui) writeonly uniform uimage2D cs_RegisteredDepth;
layout(binding = 1, r16ui) readonly uniform uimage2D cs_UnregisteredDepth;

layout (local_size_x = TILE_SIZE_2D, local_size_y = TILE_SIZE_2D, local_size_z = 1) in;
void main()
{
    ivec2 Coords = ivec2(gl_GlobalInvocationID.xy);

    uint Depth = imageLoad(cs_UnregisteredDepth, Coords).x;

    imageStore(cs_RegisteredDepth, Coords, uvec4(Depth));
}

#endif //__INCLUDE_CS_REGISTER_DEPTH_GLSL__