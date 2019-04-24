#ifndef __INCLUDE_CS_Rect_Planar_GLSL__
#define __INCLUDE_CS_Rect_Planar_GLSL__

layout(std140, binding = 0) uniform HomographyBuffer
{
    mat3 g_Homography;
};

layout (binding = 0, r8) readonly uniform image2D cs_InputImage;
layout (binding = 1, r8) writeonly uniform image2D cs_OutputImage;

layout (local_size_x = TILE_SIZE_2D, local_size_y = TILE_SIZE_2D, local_size_z = 1) in;
void main()
{
	const ivec3 pix_Rect = ivec3(gl_GlobalInvocationID.xy, 1.0f);
	ivec3 pix_Rect2Orig = inverse(HomographyBuffer->g_Homography) * pix_Rect;
	pix_Rect2Orig /= pix_Rect2Orig.z;
	BiLinearInterpolation();
	imageStore(cs_OutputImage, ivec2(gl_GlobalInvocationID.xy), pixValue_BiLinear);
}

#endif //__INCLUDE_CS_Rect_Planar_GLSL__