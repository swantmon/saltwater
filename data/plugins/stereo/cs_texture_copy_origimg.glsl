#ifndef __INCLUDE_CS_TEXTURE_COPY_ORIGIMG_GLSL__
#define __INCLUDE_CS_TEXTURE_COPY_ORIGIMG_GLSL__


layout (binding = 0, rgba8) readonly uniform image2D cs_src; // Pixel in image2D(rgba8) is 8-bit vec4 (0~255 -> 0~1)
layout (binding = 1, rgba8) writeonly uniform image2D cs_dst; // Pixel in image2D(rgba8) is 8-bit vec4 (0~255 -> 0~1)


layout (local_size_x = TILE_SIZE_2D, local_size_y = TILE_SIZE_2D, local_size_z = 1) in;
void main()
{
	ivec2 PixelPosition = ivec2(gl_GlobalInvocationID.xy);

	vec4 PixelValue = imageLoad(cs_src, PixelPosition);

	imageStore(cs_dst, PixelPosition, PixelValue); 
}

#endif //__INCLUDE_CS_TEXTURE_COPY_ORIGIMG_GLSL__