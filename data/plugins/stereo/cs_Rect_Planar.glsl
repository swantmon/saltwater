#ifndef __INCLUDE_CS_Rect_Planar_GLSL__
#define __INCLUDE_CS_Rect_Planar_GLSL__

layout(std140, binding = 0) uniform HomographyBuffer
{
    mat4 g_Homography;
	mat4 g_InvHomography;
	ivec2 g_Shift_Orig2Rect;
	ivec2 g_Padding;
};

layout (binding = 0, r8) readonly uniform image2D cs_OrigImg;
layout (binding = 1, r8) writeonly uniform image2D cs_RectImg;

float BiLinearInterpolation(vec2 pixPosition)
{
	ivec2 pixPosition_UL = ivec2(pixPosition);
	ivec2 pixPosition_UR = pixPosition_UL + ivec2(1, 0);
	ivec2 pixPosition_DL = pixPosition_UL + ivec2(0, 1);
	ivec2 pixPosition_DR = pixPosition_UL + ivec2(1, 1);

	float pixValue_UL = imageLoad(cs_OrigImg, pixPosition_UL).x;
	float pixValue_UR = imageLoad(cs_OrigImg, pixPosition_UR).x;
	float pixValue_DL = imageLoad(cs_OrigImg, pixPosition_DL).x;
	float pixValue_DR = imageLoad(cs_OrigImg, pixPosition_DR).x;

	float a_x = (pixPosition.x - pixPosition_UL.x) / (pixPosition_UR.x - pixPosition_UL.x);
	float a_y = (pixPosition.y - pixPosition_UL.y) / (pixPosition_DL.y - pixPosition_UL.y);

	float pixValue_UM = mix(pixValue_UL, pixValue_UR, a_x);
	float pixValue_DM = mix(pixValue_DL, pixValue_DR, a_x);

	float pixValue = mix(pixValue_UM, pixValue_DM, a_y);
	
	return pixValue;
}

layout (local_size_x = TILE_SIZE_2D, local_size_y = TILE_SIZE_2D, local_size_z = 1) in;
void main()
{
	const ivec3 pix_Rect = ivec3(gl_GlobalInvocationID.xy + g_Shift_Orig2Rect, 1);
	vec3 pix_Rect2Orig = mat3(g_InvHomography) * (pix_Rect);
	pix_Rect2Orig /= pix_Rect2Orig.z;

	float pixValue = BiLinearInterpolation(vec2(pix_Rect2Orig));

	//float pixValue = imageLoad(cs_OrigImg, ivec2(pix_Rect2Orig)).x; // pixValue is 0~1
	imageStore(cs_RectImg, ivec2(gl_GlobalInvocationID.xy), vec4(pixValue)); 
}

#endif //__INCLUDE_CS_Rect_Planar_GLSL__