#ifndef __INCLUDE_CS_SHIFT_DEPTH_GLSL__
#define __INCLUDE_CS_SHIFT_DEPTH_GLSL__

layout(binding = 0, r32f) readonly uniform image2D cs_ARKitDepth;
layout(binding = 1, r16ui) writeonly uniform uimage2D cs_Depth;

layout (local_size_x = TILE_SIZE_2D, local_size_y = TILE_SIZE_2D, local_size_z = 1) in;
void main()
{
	ivec2 ResizedCoords = ivec2(gl_GlobalInvocationID.xy);
    ivec2 DepthCoords = ivec2((vec2(ResizedCoords) / imageSize(cs_Depth)) * imageSize(cs_ARKitDepth));

    float Depth = imageLoad(cs_ARKitDepth, DepthCoords).x;
    imageStore(cs_Depth, ResizedCoords, ivec4(Depth * 1000));
}

#endif //__INCLUDE_CS_SHIFT_DEPTH_GLSL__