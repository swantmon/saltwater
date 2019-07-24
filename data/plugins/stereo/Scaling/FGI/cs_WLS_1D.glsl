#ifndef __INCLUDE_CS_WLS_1D_GLSL__
#define __INCLUDE_CS_WLS_1D_GLSL__

layout (binding = 0, r32f) writeonly uniform image2D cs_Out; // Pixel in image2D is 32-bit float
layout (binding = 1, r32f) readonly uniform image2D cs_In; // Pixel in image2D is 32-bit float
layout (binding = 2, r8) readonly uniform image2D cs_Guide; // Pixel in image2D is 8-bit float (0~255 -> 0~1)

layout(std140, binding = 0) uniform ParameterBuffer
{
    float g_Lamda;
	float g_Sigma;
	ivec2 g_Direction;
};

layout(std430, binding = 1) buffer ArrayCalculationBuffer
{
    vec4 g_GaussElimin[MAX_ARRAYSIZE]; // c_Temp, f_Temp, u, padding
};

float cal_Weight(ivec2 CenterPos, ivec2 NeighborPos)
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

	const int ComputeRange = int(dot(ImgSize, g_Direction));

	for (int idx = 0; idx < ComputeRange; idx++)
	{
		ivec2 Position_Curt, Position_Last, Position_Next;
		
		Position_Curt.x = g_Direction.x == 1 ? idx : int(gl_GlobalInvocationID.x);
		Position_Curt.y = g_Direction.y == 1 ? idx : int(gl_GlobalInvocationID.x);

		Position_Last = Position_Curt - g_Direction;
		Position_Next = Position_Curt + g_Direction;
		
		float a = idx == 0 ? 0.0f : -g_Lamda * cal_Weight(Position_Curt, Position_Last);
		
		float c = idx == ComputeRange-1 ? 0.0f : -g_Lamda * cal_Weight(Position_Curt, Position_Next);

		float b = 1 - a - c;
		
		float denominator = idx == 0 ? b : b - g_GaussElimin[idx-1].x * a;
		g_GaussElimin[idx].x = denominator == 0.0f ? 0.0f : c / denominator;

		float f = imageLoad(cs_In, Position_Curt).x;
		g_GaussElimin[idx].y = idx == 0 ? f / b : (f - g_GaussElimin[idx-1].y * a) / denominator;
		g_GaussElimin[idx].z = 0.0f;
	}

	g_GaussElimin[ComputeRange-1].z = g_GaussElimin[ComputeRange-1].y;
	for (int idx = ComputeRange-2; idx >= 0; idx--)
	{
		g_GaussElimin[idx].z = g_GaussElimin[idx].y - g_GaussElimin[idx].x * g_GaussElimin[idx+1].z;
	}

	for (int idx = 0; idx < ComputeRange; idx++)
	{
		ivec2 Position_Curt;
		Position_Curt.x = g_Direction.x == 1 ? idx : int(gl_GlobalInvocationID.x);
		Position_Curt.y = g_Direction.y == 1 ? idx : int(gl_GlobalInvocationID.x);

		imageStore(cs_Out, Position_Curt, vec4(g_GaussElimin[idx].z));
	}
	
}

#endif //__INCLUDE_CS_WLS_1D_GLSL__