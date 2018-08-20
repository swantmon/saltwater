#ifndef __INCLUDE_CS_YUV_TO_RGB_GLSL__
#define __INCLUDE_CS_YUV_TO_RGB_GLSL__

layout(binding = 0, r16ui) readonly uniform uimage2D cs_Shift;
layout(binding = 1, r16ui) writeonly uniform uimage2D cs_Depth;
layout(binding = 2, r16ui) readonly uniform uimage2D cs_ShiftLUT;

layout (local_size_x = TILE_SIZE_2D, local_size_y = TILE_SIZE_2D, local_size_z = 1) in;
void main()
{
    ivec2 Coords = ivec2(gl_GlobalInvocationID.xy);
    const ivec2 ImageSize = imageSize(cs_Depth);
    if (Coords.x < ImageSize.x && Coords.y < ImageSize.y)
    {
        uint Shift = imageLoad(cs_Shift, Coords).x;
        uint Depth = imageLoad(cs_ShiftLUT, ivec2(Shift, 0)).x;
        Depth = Shift < imageSize(cs_ShiftLUT).x ? Depth : 0;
        Coords.x = imageSize(cs_Depth).x - Coords.x - 1;
        imageStore(cs_Depth, Coords, uvec4(Depth));
    }
}

#endif //__INCLUDE_CS_YUV_TO_RGB_GLSL__