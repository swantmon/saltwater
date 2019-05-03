#ifndef __INCLUDE_CS_Disp_to_Depth_GLSL__
#define __INCLUDE_CS_Disp_to_Depth_GLSL__

layout(std140, binding = 0) uniform HomographyBuffer
{
    mat4 g_Homography;
	mat4 g_InvHomography;
	ivec2 g_RectImgConer_UL;
	ivec2 g_RectImgConer_DR;
};
layout(std140, binding = 1) uniform ParaxEqBuffer
{
    float g_BaselineLength;
	float g_FocalLength;
	ivec2 g_Padding;
};

layout (binding = 0, r8) readonly uniform image2D cs_Disp_Rect;
layout (binding = 1, r8) writeonly uniform image2D cs_Depth_Orig;

vec4 BiLinearInterpolation(vec2 pixPosition)
{
	ivec2 pixPosition_UL = ivec2(pixPosition);
	ivec2 pixPosition_UR = pixPosition_UL + ivec2(1, 0);
	ivec2 pixPosition_DL = pixPosition_UL + ivec2(0, 1);
	ivec2 pixPosition_DR = pixPosition_UL + ivec2(1, 1);

	vec4 pixValue_RGBA_UL = imageLoad(cs_OrigImg, pixPosition_UL);
	vec4 pixValue_RGBA_UR = imageLoad(cs_OrigImg, pixPosition_UR);
	vec4 pixValue_RGBA_DL = imageLoad(cs_OrigImg, pixPosition_DL);
	vec4 pixValue_RGBA_DR = imageLoad(cs_OrigImg, pixPosition_DR);

	vec4 pixValue_UL = vec4(0.299 * pixValue_RGBA_UL.x + 0.587 * pixValue_RGBA_UL.y + 0.114 * pixValue_RGBA_UL.z);
	vec4 pixValue_UR = vec4(0.299 * pixValue_RGBA_UR.x + 0.587 * pixValue_RGBA_UR.y + 0.114 * pixValue_RGBA_UR.z);
	vec4 pixValue_DL = vec4(0.299 * pixValue_RGBA_DL.x + 0.587 * pixValue_RGBA_DL.y + 0.114 * pixValue_RGBA_DL.z);
	vec4 pixValue_DR = vec4(0.299 * pixValue_RGBA_DR.x + 0.587 * pixValue_RGBA_DR.y + 0.114 * pixValue_RGBA_DR.z);

	float a_x = (pixPosition.x - pixPosition_UL.x) / (pixPosition_UR.x - pixPosition_UL.x);
	float a_y = (pixPosition.y - pixPosition_UL.y) / (pixPosition_DL.y - pixPosition_UL.y);

	vec4 pixValue_UM = mix(pixValue_UL, pixValue_UR, a_x);
	vec4 pixValue_DM = mix(pixValue_DL, pixValue_DR, a_x);

	vec4 pixValue = mix(pixValue_UM, pixValue_DM, a_y);
	
	return pixValue;
}

layout (local_size_x = TILE_SIZE_2D, local_size_y = TILE_SIZE_2D, local_size_z = 1) in;
void main()
{
	const ivec3 pixPosition_Orig = ivec3(gl_GlobalInvocationID.xy, 1);
	vec3 pixPosition_Orig2Rect = mat3(g_Homography) * (pixPosition_Orig);
	pixPosition_Orig2Rect /= pixPosition_Orig2Rect.z;
	vec2 pixPosition_Rect = vec2(pixPosition_Orig2Rect) - g_RectImgConer_UL;
    
    

	//*** OLD ***
	const ivec3 pix_Rect = ivec3(gl_GlobalInvocationID.xy + g_RectImgConer_UL, 1);
	vec3 pix_Rect2Orig = mat3(g_InvHomography) * (pix_Rect);
	pix_Rect2Orig /= pix_Rect2Orig.z;

	vec4 pixValue = BiLinearInterpolation(vec2(pix_Rect2Orig));

	//float pixValue = imageLoad(cs_OrigImg, ivec2(pix_Rect2Orig)).x; // pixValue is 0~1
	imageStore(cs_RectImg, ivec2(gl_GlobalInvocationID.xy), pixValue); 
}

#endif //__INCLUDE_CS_Disp_to_Depth_GLSL__