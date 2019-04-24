#ifndef __INCLUDE_CS_Rect_Planar_GLSL__
#define __INCLUDE_CS_Rect_Planar_GLSL__

layout(std140, binding = 0) uniform HomographyBuffer
{
    mat4 g_Homography;
	mat4 g_InvHomography;
	ivec2 g_RectImgConer_UL;
	ivec2 g_RectImgConer_DR;
};

layout (binding = 0, r8) readonly uniform image2D cs_InputImage;
layout (binding = 1, r8) writeonly uniform image2D cs_OutputImage;

int BiLinearInterpolation(const ivec2 pixPosition, const ivec2 ImgSize)
{
	if (pixPosition.x < 0 || pixPosition.y < 0 || pixPosition.x > (ImgSize.x - 1) || pixPosition.y > (ImgSize.y - 1))
	{
		pixValue = null;
	}
	else if ()
	{
		cs_InputImage[]
	}
	
	int pixValue = int(
	    pixValue_UL * (1 - pix_x) * (1 - pix_y) + 
		pixValue_UR * (pix_x) * (1 - pix_y) +
		pixValue_DL * (1 - pix_x) * (pix_y) +
		pixValue_DR * (pix_x) * (pix_y)
	);
	return pixValue
}

layout (local_size_x = TILE_SIZE_2D, local_size_y = TILE_SIZE_2D, local_size_z = 1) in;
void main()
{
	const ivec3 pix_Rect = ivec3(gl_GlobalInvocationID.xy, 1);
	vec3 pix_Rect2Orig = mat3(g_InvHomography) * pix_Rect;
	pix_Rect2Orig /= pix_Rect2Orig.z;
	const ivec2 ImgSize = imageSize(cs_InputImage);
	BiLinearInterpolation();
	imageStore(cs_OutputImage, ivec2(gl_GlobalInvocationID.xy), pixValue_BiLinear);
}

#endif //__INCLUDE_CS_Rect_Planar_GLSL__