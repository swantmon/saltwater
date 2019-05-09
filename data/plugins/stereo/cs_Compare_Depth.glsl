#ifndef __INCLUDE_CS_Compare_Depth_GLSL__
#define __INCLUDE_CS_Compare_Depth_GLSL__

layout (binding = 0, r16ui) readonly uniform uimage2D cs_Depth_StereoMatching;
layout (binding = 1, r16ui) readonly uniform uimage2D cs_Depth_Sensor;
layout (binding = 2, r16ui) writeonly uniform uimage2D cs_Depth_Difference;

layout (local_size_x = TILE_SIZE_2D, local_size_y = TILE_SIZE_2D, local_size_z = 1) in;
void main()
{
	uint Depth_Diff = imageLoad(cs_Depth_StereoMatching, ivec2(gl_GlobalInvocationID.xy)).r - imageLoad(cs_Depth_Sensor, ivec2(gl_GlobalInvocationID.xy)).r;
	imageStore(cs_Depth_Difference, ivec2(gl_GlobalInvocationID.xy), uvec4(Depth_Diff));
}

#endif //__INCLUDE_CS_Compare_Depth_GLSL__