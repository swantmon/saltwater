#ifndef __INCLUDE_CS_Rect_Planar_GLSL__
#define __INCLUDE_CS_Rect_Planar_GLSL__

layout(std140, binding = 0) uniform HomographyBuffer
{
    mat4 g_Homography;
	mat4 g_InvHomography;
	ivec2 g_RectImgConer_UL;
	ivec2 g_RectImgConer_DR;
};

layout (binding = 0, r16ui) readonly uniform uimage2D cs_Depth_RectImg; // Pixel in uimage2D(r16ui) is uint in 16-bit
layout (binding = 1, r16ui) writeonly uniform uimage2D cs_Depth_OrigImg; // Pixel in uimage2D(r16ui) is uint in 16-bit

float BiLinearInterpolation(vec2 pixPosition)
{
	const ivec2 pixPosition_UL = ivec2(pixPosition);
	const ivec2 pixPosition_UR = pixPosition_UL + ivec2(1, 0);
	const ivec2 pixPosition_DL = pixPosition_UL + ivec2(0, 1);
	const ivec2 pixPosition_DR = pixPosition_UL + ivec2(1, 1);

	const uint pixValue_UL = imageLoad(cs_Depth_RectImg, pixPosition_UL).r;
	const uint pixValue_UR = imageLoad(cs_Depth_RectImg, pixPosition_UR).r;
	const uint pixValue_DL = imageLoad(cs_Depth_RectImg, pixPosition_DL).r;
	const uint pixValue_DR = imageLoad(cs_Depth_RectImg, pixPosition_DR).r;

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
	const ivec3 pix_Orig = ivec3(gl_GlobalInvocationID.xy, 1);
	vec3 pix_Orig2Rect = mat3(g_Homography) * (pix_Orig);
	pix_Orig2Rect /= pix_Orig2Rect.z;
	vec2 pix_Rect = vec2(pix_Orig2Rect) - g_RectImgConer_UL;

	float Depth = BiLinearInterpolation(pix_Rect);

	ivec2 Coords = ivec2(gl_GlobalInvocationID.xy);
	//Coords.x = imageSize(cs_Depth_OrigImg).x - Coords.x;
	imageStore(cs_Depth_OrigImg, Coords, uvec4(Depth)); 
}

#endif //__INCLUDE_CS_Rect_Planar_GLSL__