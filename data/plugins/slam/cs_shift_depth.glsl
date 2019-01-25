#ifndef __INCLUDE_CS_YUV_TO_RGB_GLSL__
#define __INCLUDE_CS_YUV_TO_RGB_GLSL__

layout(binding = 0, r16ui) readonly uniform uimage2D cs_Shift;
layout(binding = 1, r16ui) writeonly uniform uimage2D cs_Depth;
layout(binding = 2, r16ui) readonly uniform uimage2D cs_ShiftLUT;

layout (local_size_x = TILE_SIZE_2D, local_size_y = TILE_SIZE_2D, local_size_z = 1) in;
void main()
{
#ifdef CAPTURE_COLOR

    ivec2 Coords = ivec2(gl_GlobalInvocationID.xy);
    if (Coords.x < COLOR_WIDTH && Coords.y < COLOR_HEIGHT)
    {
        Coords.y += (DEPTH_HEIGHT - COLOR_HEIGHT) / 2;
        uint Shift = imageLoad(cs_Shift, Coords).x;
        uint Depth = imageLoad(cs_ShiftLUT, ivec2(Shift, 0)).x;
        Depth = Shift < imageSize(cs_ShiftLUT).x ? Depth : 0;
        Coords.y -= (DEPTH_HEIGHT - COLOR_HEIGHT) / 2;
        Coords.x = COLOR_WIDTH - Coords.x - 1;
        imageStore(cs_Depth, Coords, uvec4(Depth));
    }

#else

    ivec2 Coords = ivec2(gl_GlobalInvocationID.xy);
    if (Coords.x < DEPTH_WIDTH && Coords.y < DEPTH_HEIGHT)
    {
        uint Shift = imageLoad(cs_Shift, Coords).x;
        uint Depth = imageLoad(cs_ShiftLUT, ivec2(Shift, 0)).x;
        Depth = Shift < imageSize(cs_ShiftLUT).x ? Depth : 0;
        Coords.x = imageSize(cs_Depth).x - Coords.x - 1;
        imageStore(cs_Depth, Coords, uvec4(Depth));
    }

#endif
}

#endif //__INCLUDE_CS_YUV_TO_RGB_GLSL__