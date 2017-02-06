
#ifndef __INCLUDE_CS_KINECT_RAYCAST_PYRAMID_GLSL__
#define __INCLUDE_CS_KINECT_RAYCAST_PYRAMID_GLSL__

// -----------------------------------------------------------------------------
// Constants
// -----------------------------------------------------------------------------

layout(row_major, std140, binding = 0) uniform UBONormalize
{
	bool g_Normalize;
};

// -----------------------------------------------------------------------------
// Input from engine

// -----------------------------------------------------------------------------
layout (binding = 0, rgba32f) readonly uniform image2D cs_InputTexture;
layout (binding = 1, rgba32f) writeonly uniform image2D cs_OutputTexture;

// Functions
// -------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------
layout (local_size_x = TILE_SIZE2D, local_size_y = TILE_SIZE2D, local_size_z = 1) in;
void main()
{
	const vec2 InputImageSize = imageSize(cs_InputTexture);
	const vec2 OutputImageSize = imageSize(cs_OutputTexture);

	const vec2 SamplePos = gl_GlobalInvocationID.xy / OutputImageSize * InputImageSize;
	
	vec4 Sum = vec4(0.0f);
	
	bool IsInvalid = false;
	
	for (int i = 0; i < 1; ++ i)
	{
		for (int j = 0; j < 1; ++ j)
		{
			const vec4 Sample = imageLoad(cs_InputTexture, ivec2(SamplePos) + ivec2(i, j));
			
			if (Sample.x == 0.0f)
			{
				IsInvalid = true;
				break;
			}
			
			Sum += Sample;
		}
	}

	vec4 Result = Sum / 4.0f;
	
	if (g_Normalize)
	{
		Result.xyz = normalize(Result.xyz);
	}
	
	imageStore(cs_OutputTexture, ivec2(gl_GlobalInvocationID), IsInvalid ? vec4(0.0f) : Result);
}

#endif // __INCLUDE_CS_KINECT_RAYCAST_PYRAMID_GLSL__