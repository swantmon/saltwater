#ifndef __INCLUDE_CS_Disparity_to_Depth_GLSL__
#define __INCLUDE_CS_Disparity_to_Depth_GLSL__

layout(std140, binding = 0) uniform ParaxEqBuffer
{
    float g_BaselineLength;
	float g_FocalLength;
	ivec2 g_Padding;
};

layout (binding = 0, r32f) readonly uniform image2D cs_Disp_RectImg; // Pixel in image2D(r32f) is float in 32-bit
layout (binding = 1, r16ui) writeonly uniform uimage2D cs_Depth_RectImg; // Pixel in uimage2D(r16ui) is uint in 16-bit

layout (local_size_x = TILE_SIZE_2D, local_size_y = TILE_SIZE_2D, local_size_z = 1) in;
void main()
{
	float Disparity = imageLoad(cs_Disp_RectImg, ivec2(gl_GlobalInvocationID.xy)).r;
	
	float Depth = Disparity == 0.0f ? 0.0f : g_FocalLength * g_BaselineLength / Disparity;
	Depth = Depth < 4 ? Depth : 0; // Depth more than 4 m is regarded as outlier.
	Depth *= 1000.0f; // Unit = mini-meter
	imageStore(cs_Depth_RectImg, ivec2(gl_GlobalInvocationID.xy), uvec4(Depth));
}

#endif //__INCLUDE_CS_Disparity_to_Depth_GLSL__