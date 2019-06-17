#ifndef __INCLUDE_CS_Rectification_Planar_GLSL__
#define __INCLUDE_CS_Rectification_Planar_GLSL__

layout(std140, binding = 0) uniform HomographyBuffer
{
    mat4 g_Homography;
	mat4 g_InvHomography;
	ivec2 g_RectImgConer_UL;
	ivec2 g_RectImgConer_DR;
};

layout (binding = 0, rgba8) readonly uniform image2D cs_OrigImg; // Pixel in image2D(rgba8) is 8-bit vec4 (0~255 -> 0~1)
layout (binding = 1, r8) writeonly uniform image2D cs_RectImg; // Pixel in image2D(r8) is 8-bit float (0~255 -> 0~1)

float BiLinearInterpolation(vec2 pixPosition)
{
	const ivec2 pixPosition_UL = ivec2(pixPosition);
	const ivec2 pixPosition_UR = pixPosition_UL + ivec2(1, 0);
	const ivec2 pixPosition_DL = pixPosition_UL + ivec2(0, 1);
	const ivec2 pixPosition_DR = pixPosition_UL + ivec2(1, 1);

	const vec4 pixValue_RGBA_UL = imageLoad(cs_OrigImg, pixPosition_UL);
	const vec4 pixValue_RGBA_UR = imageLoad(cs_OrigImg, pixPosition_UR);
	const vec4 pixValue_RGBA_DL = imageLoad(cs_OrigImg, pixPosition_DL);
	const vec4 pixValue_RGBA_DR = imageLoad(cs_OrigImg, pixPosition_DR);

	const float pixValue_UL = 0.299 * pixValue_RGBA_UL.x + 0.587 * pixValue_RGBA_UL.y + 0.114 * pixValue_RGBA_UL.z;
	const float pixValue_UR = 0.299 * pixValue_RGBA_UR.x + 0.587 * pixValue_RGBA_UR.y + 0.114 * pixValue_RGBA_UR.z;
	const float pixValue_DL = 0.299 * pixValue_RGBA_DL.x + 0.587 * pixValue_RGBA_DL.y + 0.114 * pixValue_RGBA_DL.z;
	const float pixValue_DR = 0.299 * pixValue_RGBA_DR.x + 0.587 * pixValue_RGBA_DR.y + 0.114 * pixValue_RGBA_DR.z;

	const float a_x = (pixPosition.x - pixPosition_UL.x) / (pixPosition_UR.x - pixPosition_UL.x);
	const float a_y = (pixPosition.y - pixPosition_UL.y) / (pixPosition_DL.y - pixPosition_UL.y);

	const float pixValue_UM = mix(pixValue_UL, pixValue_UR, a_x);
	const float pixValue_DM = mix(pixValue_DL, pixValue_DR, a_x);

	const float pixValue = mix(pixValue_UM, pixValue_DM, a_y);
	
	return pixValue;
}

layout (local_size_x = TILE_SIZE_2D, local_size_y = TILE_SIZE_2D, local_size_z = 1) in;
void main()
{
	const ivec3 pix_Rect = ivec3(gl_GlobalInvocationID.xy + g_RectImgConer_UL, 1);
	vec3 pix_Rect2Orig = mat3(g_InvHomography) * (pix_Rect);
	pix_Rect2Orig /= pix_Rect2Orig.z;

	float pixValue = BiLinearInterpolation(vec2(pix_Rect2Orig));

	imageStore(cs_RectImg, ivec2(gl_GlobalInvocationID.xy), vec4(pixValue)); 
}

#endif //__INCLUDE_CS_Rectification_Planar_GLSL__