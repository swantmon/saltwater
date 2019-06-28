#ifndef __INCLUDE_CS_Up_Sampling_GLSL__
#define __INCLUDE_CS_Up_Sampling_GLSL__

layout (binding = 0, r32f) readonly uniform image2D cs_Disp_DownSample; // Pixel in image2D is 32-bit float
layout (binding = 1, r32f) writeonly uniform image2D cs_Disp_UpSample; // Pixel in image2D is 32-bit float

float BiLinearInterpolation(vec2 pixPosition)
{
	const ivec2 pixPosition_UL = ivec2(pixPosition);
	const ivec2 pixPosition_UR = pixPosition_UL + ivec2(1, 0);
	const ivec2 pixPosition_DL = pixPosition_UL + ivec2(0, 1);
	const ivec2 pixPosition_DR = pixPosition_UL + ivec2(1, 1);

	const float pixValue_UL = imageLoad(cs_Disp_DownSample, pixPosition_UL).x;
	const float pixValue_UR = imageLoad(cs_Disp_DownSample, pixPosition_UR).x;
	const float pixValue_DL = imageLoad(cs_Disp_DownSample, pixPosition_DL).x;
	const float pixValue_DR = imageLoad(cs_Disp_DownSample, pixPosition_DR).x;

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
	const ivec2 ImgSize_DownSample = imageSize(cs_Disp_DownSample);
	const ivec2 ImgSize_UpSample = imageSize(cs_Disp_UpSample);

	const vec2 UpRatio = ImgSize_DownSample / ImgSize_UpSample;

	const vec2 pix_Sample = UpRatio * uvec2(gl_GlobalInvocationID.xy);

	float pixValue = BiLinearInterpolation(pix_Sample);

	imageStore(cs_Disp_UpSample, ivec2(gl_GlobalInvocationID.xy), vec4(pixValue)); 
}

#endif //__INCLUDE_CS_Up_Sampling_GLSL__