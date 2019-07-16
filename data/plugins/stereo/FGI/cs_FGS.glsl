#ifndef __INCLUDE_CS_FGS_GLSL__
#define __INCLUDE_CS_FGS_GLSL__

layout (binding = 0, r32f) writeonly uniform image2D cs_Out; // Pixel in image2D is 32-bit float
layout (binding = 1, r32f) readonly uniform image2D cs_In; // Pixel in image2D is 32-bit float
layout (binding = 2, r32f) readonly uniform image2D cs_Guide; // Pixel in image2D is 32-bit float

layout(std140, binding = 0) uniform ParameterBuffer
{
    float g_Lamda;
	float g_Sigma;
	ivec2 g_Direction;
};

float cal_GuidedWeight(ivec2 CenterPos, ivec2 NeighborPos)
{
	const float CenterPixValue = imageLoad(cs_Guide, CenterPos).x;
	const float NeighborPixValue = imageLoad(cs_Guide, NeighborPos).x;

	const float GuidedSimilarity = sqrt(pow(CenterPixValue - NeighborPixValue, 2));
	const float GuidedWeight = exp(-GuidedSimilarity / g_Sigma.x);

	return GuidedWeight;
}

layout (local_size_x = TILE_SIZE_1D, local_size_y = 1, local_size_z = 1) in;
void main()
{
	const ivec2 ImgSize = imageSize(cs_Out);

	int ComputeRange = int(dot(ImgSize, g_Direction));

	float a[ComputeRange], b[ComputeRange], c[ComputeRange];
	float c_Temp[ComputeRange], f_Temp[ComputeRange];

	for (int idx = 0; idx < ComputeRange; idx++)
	{
		if (idx == 0)
		{
			a[idx] = 0.0f;
		}
		else
		{
			
		}

		if (idx == ComputeRange-1)
		{
			c[idx] = 0.0f;
		}
		else
		{
			
		}

		b[idx] = 1 - a[idx] - c[idx];
		

	}

	//***OLD***
	
	for (int iter = 0; iter < Iteration_Number; iter++)
	{
		//---Horizontal 1D WLS---
		const int y_wise = int(gl_GlobalInvocationID.y); 

		for (int x = 0; x < UpSampleSize.x; x++)
		{
			if (x == 0)
			{
				a_x[x] = 0.0f;
			}
			else
			{
				const float Weight_left = cal_GuidedWeight(ivec2(x, y_wise), ivec2(x-1, y_wise));

				a_x[x] = -Lamda_t * Weight_left;
			}
			
			if (x == UpSampleSize.x-1)
			{
				c_x[x] = 0.0f;
			}
			else
			{
				const float Weight_right = cal_GuidedWeight(ivec2(x, y_wise), ivec2(x+1, y_wise));
			
				c_x[x] = -Lamda_t * Weight_right;
			}

			b_x[x] = 1 - a_x[x] - c_x[x];

			if (x == 0)
			{
				c_x_Temp[x] = c_x[x] / b_x[x];

				const float f_x_Curt = imageLoad(cs_In, ivec2(x, y_wise)).x;
				f_x_Temp[x] = f_x_Curt / b_x[x];
			}
			else
			{
				c_x_Temp[x] = c_x[x] / (b_x[x] - c_x_Temp[x-1] * a_x[x]);

				const float f_x_Curt = imageLoad(cs_In, ivec2(x, y_wise)).x;
				const float f_x_Last = imageLoad(cs_In, ivec2(x-1, y_wise)).x;
				f_x_Temp[x] = (f_x_Curt - f_x_Last * a_x[x]) / (b_x[x] - c_x_Temp[x-1] * a_x[x]);
			}
			
		}

		imageStore(cs_Out, ivec2(UpSampleSize.x-1, y_wise), vec4(f_x_Temp[UpSampleSize.x-1]));
		for (int x = UpSampleSize.x-2; x >= 0; x--)
		{
			const float u_x = f_x_Temp[x] - c_x_Temp[x] * imageLoad(cs_Out, ivec2(x+1, y_wise));
			imageStore(cs_Out, ivec2(x, y_wise), vec4(u_x));
		}

		//---Vertical 1D WLS---
		float a_y[UpSampleSize.y], b_y[UpSampleSize.y], c_y[UpSampleSize.y];
		float c_y_Temp[UpSampleSize.y], f_y_Temp[UpSampleSize.y];

		const int x_Wise = int(gl_GlobalInvocationID.x); 

		for (int y = 0; y < UpSampleSize.y; y++)
		{
			if (y == 0)
			{
				a_y[y] = 0.0f;
			}
			else
			{
				const float Weight_up = cal_GuidedWeight(ivec2(x_Wise, y), ivec2(x_Wise, y-1));

				a_y[y] = -Lamda_t * Weight_up;
			}
			
			if (y == UpSampleSize.y-1)
			{
				c_y[y] = 0.0f;
			}
			else
			{
				const float Weight_down = cal_GuidedWeight(ivec2(x_Wise, y), ivec2(x_Wise, y+1));
			
				c_y[y] = -Lamda_t * Weight_down;
			}

			b_y[y] = 1 - a_y[y] - c_y[y];

			if (y == 0)
			{
				c_y_Temp[y] = c_y[y] / b_y[y];

				const float f_y_Curt = imageLoad(cs_In, ivec2(x_Wise, y)).x;
				f_y_Temp[y] = f_y_Curt / b_y[y];
			}
			else
			{
				c_y_Temp[y] = c_y[y] / (b_y[y] - c_y_Temp[y-1] * a_y[y]);

				const float f_y_Curt = imageLoad(cs_In, ivec2(x_Wise, y)).x;
				const float f_y_Last = imageLoad(cs_In, ivec2(x_Wise, y-1)).x;
				f_y_Temp[y] = (f_y_Curt - f_y_Last * a_y[y]) / (b_y[y] - c_y_Temp[y-1] * a_y[y]);
			}
			
		}

		imageStore(cs_Out, ivec2(x_Wise, UpSampleSize.y-1), vec4(f_y_Temp[UpSampleSize.y-1]));
		for (int y = UpSampleSize.y-2; y >= 0; y--)
		{
			const float u_y = f_y_Temp[y] - c_y_Temp[y] * imageLoad(cs_Out, ivec2(x_Wise, y+1));
			imageStore(cs_Out, ivec2(x_Wise, y), vec4(u_y));
		}
	}
	
}

#endif //__INCLUDE_CS_FGS_GLSL__