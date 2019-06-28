#ifndef __INCLUDE_CS_Down_Sampling_GLSL__
#define __INCLUDE_CS_Down_Sampling_GLSL__

layout (binding = 0, r8) readonly uniform image2D cs_Img_OrigScale; // Pixel in image2D(rgba8) is 8-bit float (0~255 -> 0~1)
layout (binding = 1, r8) writeonly uniform image2D cs_Img_DownSample; // Pixel in image2D(r8) is 8-bit float (0~255 -> 0~1)

float BiLinearInterpolation(vec2 pixPosition)
{
	const ivec2 pixPosition_UL = ivec2(pixPosition);
	const ivec2 pixPosition_UR = pixPosition_UL + ivec2(1, 0);
	const ivec2 pixPosition_DL = pixPosition_UL + ivec2(0, 1);
	const ivec2 pixPosition_DR = pixPosition_UL + ivec2(1, 1);

	const float pixValue_UL = imageLoad(cs_Img_OrigScale, pixPosition_UL).x;
	const float pixValue_UR = imageLoad(cs_Img_OrigScale, pixPosition_UR).x;
	const float pixValue_DL = imageLoad(cs_Img_OrigScale, pixPosition_DL).x;
	const float pixValue_DR = imageLoad(cs_Img_OrigScale, pixPosition_DR).x;

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
	const vec2 ImgSize_OrigScale = imageSize(cs_Img_OrigScale);
	const vec2 ImgSize_DownSample = imageSize(cs_Img_DownSample);

	const vec2 Sample = ImgSize_OrigScale / ImgSize_DownSample;

	const vec2 pix_Sample = Sample * uvec2(gl_GlobalInvocationID.xy);

	float pixValue = BiLinearInterpolation(pix_Sample);

	imageStore(cs_Img_DownSample, ivec2(gl_GlobalInvocationID.xy), vec4(pixValue)); 
}

#endif //__INCLUDE_CS_Down_Sampling_GLSL__