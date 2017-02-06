
#ifndef __INCLUDE_CS_KINECT_DOWNSAMPLE_DEPTH_GLSL__
#define __INCLUDE_CS_KINECT_DOWNSAMPLE_DEPTH_GLSL__

// -----------------------------------------------------------------------------
// Constants
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Input from engine
// -----------------------------------------------------------------------------

layout (binding = 0, r16ui) readonly uniform uimage2D cs_InputTexture;
layout (binding = 1, r16ui) writeonly uniform uimage2D cs_OutputTexture;

// -------------------------------------------------------------------------------------
// Functions
// -------------------------------------------------------------------------------------
layout (local_size_x = TILE_SIZE2D, local_size_y = TILE_SIZE2D, local_size_z = 1) in;
void main()
{
	const vec2 InputImageSize = imageSize(cs_InputTexture);
	const vec2 OutputImageSize = imageSize(cs_OutputTexture);

	const vec2 SamplePos = gl_GlobalInvocationID.xy / OutputImageSize * InputImageSize;
	
	int Sum = 0;

	for (int i = -2; i < 3; ++ i)
	{
		for (int j = -2; j < 3; ++ j)
		{
			const int SampleDepth = int(imageLoad(cs_InputTexture, ivec2(SamplePos) + ivec2(i, j)).x);
			
			Sum += SampleDepth;
		}
	}

	const int Result = Sum / 25;
	
	imageStore(cs_OutputTexture, ivec2(gl_GlobalInvocationID), ivec4(Result));
}

#endif // __INCLUDE_CS_KINECT_DOWNSAMPLE_DEPTH_GLSL__