#ifndef __INCLUDE_CS_FGS_GLSL__
#define __INCLUDE_CS_FGS_GLSL__

layout (binding = 0, r32f) writeonly uniform image2D cs_Out; // Pixel in image2D is 32-bit float
layout (binding = 1, r32f) readonly uniform image2D cs_In; // Pixel in image2D is 32-bit float
layout (binding = 2, r32f) readonly uniform image2D cs_Guide; // Pixel in image2D is 32-bit float

layout(std140, binding = 0) uniform ParameterBuffer
{
    vec2 g_Lamda;
	vec2 g_Sigma;
};

float cal_GuidedWeight(ivec2 CenterPos, ivec2 NeighborPos)
{
	float CenterValue = imageLoad(cs_Guide, CenterPos).x;
	float NeighborValue = imageLoad(cs_Guide, NeighborPos).x;

	float GuidedSimilarity = sqrt(pow(CenterValue - NeighborValue, 2));
	float GuidedWeight = exp(-GuidedSimilarity / g_Sigma.x);

	return GuidedWeight;
}

layout (local_size_x = TILE_SIZE_2D, local_size_y = TILE_SIZE_2D, local_size_z = 1) in;
void main()
{
	const ivec2 UpSampleSize = imageSize(cs_Out);

	//---Horizontal 1D WLS---
	float Weight_left, Weight_right, Weight_up, Weight_down;

	float a_x[UpSampleSize.x], b_x[UpSampleSize.x], c_x[UpSampleSize.x];
	float c_x_Temp[UpSampleSize.x], f_x_Temp[UpSampleSize.x];

	int y_wise = int(gl_GlobalInvocationID.y); 

	for (int x = 0; x < UpSampleSize.x; x++)
	{
		Weight_left = cal_GuidedWeight(ivec2(x, y_wise), ivec2(x-1, y_wise));
		Weight_right = cal_GuidedWeight(ivec2(x, y_wise), ivec2(x+1, y_wise));

		a_x[x] = -g_Lamda.x * Weight_left;
		c_x[x] = -g_Lamda.x * Weight_right;
		b_x[x] = 1 - a_x[x] - c_x[x];

		c_x_Temp[x] = c_x[x] / (b_x[x] - c_x_Temp[x-1] * a_x[x]);
		f_x_Temp[x] = (f_x[x] - f_x[x-1] * a_x[x]) / (b_x[x] - c_x_Temp[x-1] * a_x[x]);
	}

	for (int x = UpSampleSize.x - 2; x = 0; x--)
	{
		u[x] = f_x_Temp[x] - c_x_Temp[x] * u[x+1];
	}

	//---Vertical 1D WLS---
	int ColWise = int(gl_GlobalInvocationID.x);
}

#endif //__INCLUDE_CS_FGS_GLSL__