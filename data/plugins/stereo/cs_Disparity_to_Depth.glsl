#ifndef __INCLUDE_CS_Disparity_to_Depth_GLSL__
#define __INCLUDE_CS_Disparity_to_Depth_GLSL__

layout(std140, binding = 0) uniform ParaxEqBuffer
{
    float g_BaselineLength;
	float g_FocalLength;
	ivec2 g_Padding;
};

layout (binding = 0, r32) readonly uniform image2D cs_Disp_RectImg;
layout (binding = 1, r32) writeonly uniform image2D cs_Depth_RectImg;

layout (local_size_x = TILE_SIZE_2D, local_size_y = TILE_SIZE_2D, local_size_z = 1) in;
void main()
{
	float Disparity = imageLoad(cs_Disp_RectImg, ivec2(gl_GlobalInvocationID.xy));
	float Depth = g_FocalLength * g_BaselineLength / Disparity;
	imageStore(cs_Depth_RectImg, ivec2(gl_GlobalInvocationID.xy), Depth);
}

#endif //__INCLUDE_CS_Disparity_to_Depth_GLSL__