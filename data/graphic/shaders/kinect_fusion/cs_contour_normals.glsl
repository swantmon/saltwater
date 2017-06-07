
#ifndef __INCLUDE_CS_CONTOUR_NORMALS_GLSL__
#define __INCLUDE_CS_CONTOUR_NORMALS_GLSL__

#include "common_tracking.glsl"

// -----------------------------------------------------------------------------
// Defines
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Input from engine
// -----------------------------------------------------------------------------

layout(binding = 0, r16ui) uniform uimage2D cs_DepthBuffer;
layout(binding = 1, MAP_TEXTURE_FORMAT) uniform image2D cs_NormalBuffer;
layout(binding = 2, MAP_TEXTURE_FORMAT) uniform image2D cs_ReferenceNormalBuffer;

// -------------------------------------------------------------------------------------
// Functions
// -------------------------------------------------------------------------------------

const int g_KernelSize = 7;

/*float g_SobelKernel[g_KernelSize * g_KernelSize] = {
    -0.25f, -0.20f, 0.0f, 0.20f, 0.25f,
    -0.20f, -0.50f, 0.0f, 0.50f, 0.20f,
    -0.50f, -1.00f, 0.0f, 1.00f, 0.50f,
    -0.20f, -0.50f, 0.0f, 0.50f, 0.20f,
    -0.25f, -0.20f, 0.0f, 0.20f, 0.25f,
};*/

float g_SobelKernel[g_KernelSize * g_KernelSize] = {
    -3.0f / 18.0f, -2.0f / 13.0f, -1.0f / 10.0f, 0.0f, 1.0f / 10.0f, 2.0f / 13.0f, 3.0f / 18.0f,
    -3.0f / 13.0f, -2.0f /  8.0f, -1.0f /  5.0f, 0.0f, 1.0f /  5.0f, 2.0f /  8.0f, 3.0f / 13.0f,
    -3.0f / 10.0f, -2.0f /  5.0f, -1.0f /  2.0f, 0.0f, 1.0f /  2.0f, 2.0f /  5.0f, 3.0f / 10.0f,
    -3.0f /  9.0f, -2.0f /  4.0f, -1.0f /  1.0f, 0.0f, 1.0f /  1.0f, 2.0f /  4.0f, 3.0f /  9.0f,
    -3.0f / 10.0f, -2.0f /  5.0f, -1.0f /  2.0f, 0.0f, 1.0f /  2.0f, 2.0f /  5.0f, 3.0f / 10.0f,
    -3.0f / 13.0f, -2.0f /  8.0f, -1.0f /  5.0f, 0.0f, 1.0f /  5.0f, 2.0f /  8.0f, 3.0f / 13.0f,
    -3.0f / 18.0f, -2.0f / 13.0f, -1.0f / 10.0f, 0.0f, 1.0f / 10.0f, 2.0f / 13.0f, 3.0f / 18.0f,
};

vec2 ComputeGradient(ivec2 Position)
{
	vec2 Result = vec2(0.0f);

	for (int i = 0; i < g_KernelSize; ++i)
	{
		for (int j = 0; j < g_KernelSize; ++j)
		{
			ivec2 SampleOffset = ivec2(i, j) - g_KernelSize / 2;

			const float Sample = float(imageLoad(cs_DepthBuffer, Position + SampleOffset));

			Result.x += Sample * g_SobelKernel[j * g_KernelSize + i];
			Result.y -= Sample * g_SobelKernel[i * g_KernelSize + j];
		}
	}

	return Result / 1000.0f;
}

layout(local_size_x = TILE_SIZE2D, local_size_y = TILE_SIZE2D, local_size_z = 1) in;
void main()
{
	const vec2 ImageSize = imageSize(cs_DepthBuffer);

	const int u = int(gl_GlobalInvocationID.x);
	const int v = int(gl_GlobalInvocationID.y);

	const int PyramidLevel = int(log2(DEPTH_IMAGE_WIDTH / ImageSize.x));

	const vec2 Gradient = ComputeGradient(ivec2(u, v));

	const int Depth = int(imageLoad(cs_DepthBuffer, ivec2(u, v)).x);
	vec3 Normal = vec3(0.0f);

	const vec2 uv = vec2(u, v) / ImageSize;

	if (Depth != 0)
	{
		const float z = Depth / 1000.0f;

		const vec2 FocalPoint = g_Intrinisics[PyramidLevel].m_FocalPoint / ImageSize;
		const vec2 FocalLength = g_Intrinisics[PyramidLevel].m_FocalLength / ImageSize;

		const vec2 Temp = Gradient * (uv - FocalPoint);

		Normal.xy = (Gradient * FocalLength) / z;
		Normal.z = (Temp.y - Temp.x) / z;

		//Normal = mat3(g_PoseMatrix) * Normal;
	}

	const vec3 Reference = imageLoad(cs_ReferenceNormalBuffer, ivec2(u, v)).xyz;

	//imageStore(cs_NormalBuffer, ivec2(u, v), vec4(Gradient, length(Gradient), 1.0f));
	//imageStore(cs_NormalBuffer, ivec2(u, v), vec4(Normal, length(Normal)));
	imageStore(cs_NormalBuffer, ivec2(u, v), vec4(normalize(Normal), 1.0f));
	//imageStore(cs_NormalBuffer, ivec2(u, v), vec4(Normal.xy, 0.0f, 1.0f));
	imageStore(cs_ReferenceNormalBuffer, ivec2(u, v), vec4(abs(Reference - Normal), 1.0f));
}

#endif // __INCLUDE_CS_CONTOURS_NORMAL_GLSL__