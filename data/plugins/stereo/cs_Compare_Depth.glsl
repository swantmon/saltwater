#ifndef __INCLUDE_CS_Compare_Depth_GLSL__
#define __INCLUDE_CS_Compare_Depth_GLSL__

layout (binding = 0, r16ui) readonly uniform uimage2D cs_Depth_Vision; // Pixel in uimage2D(r16ui) is uint in 16-bit
layout (binding = 1, r16ui) readonly uniform uimage2D cs_Depth_Sensor; // Pixel in uimage2D(r16ui) is uint in 16-bit
layout (binding = 2, r16i) writeonly uniform iimage2D cs_Depth_Difference; // Pixel in iimage2D(r16i) is int in 16-bit

layout (local_size_x = TILE_SIZE_2D, local_size_y = TILE_SIZE_2D, local_size_z = 1) in;
void main()
{
	const uint depth_Sensor = imageLoad(cs_Depth_Sensor, ivec2(gl_GlobalInvocationID.xy)).r;
	const uint depth_Stereo = imageLoad(cs_Depth_Vision, ivec2(gl_GlobalInvocationID.xy)).r;
	int depth_Diff = int(depth_Stereo) - int(depth_Sensor);
	imageStore(cs_Depth_Difference, ivec2(gl_GlobalInvocationID.xy), ivec4(depth_Diff));
}

#endif //__INCLUDE_CS_Compare_Depth_GLSL__