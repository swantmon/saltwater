
#ifndef __INCLUDE_CS_KINECT_BILATERAL_FILTER_GLSL__
#define __INCLUDE_CS_KINECT_BILATERAL_FILTER_GLSL__

// -----------------------------------------------------------------------------
// Defines
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Input from engine
// -----------------------------------------------------------------------------

layout (binding = 0, r16ui) readonly uniform uimage2D cs_InputTexture;
layout (binding = 1, r16ui) writeonly uniform uimage2D cs_OutputTexture;

// -------------------------------------------------------------------------------------
// Functions
// -------------------------------------------------------------------------------------
layout (local_size_x = TILE_SIZE, local_size_y = TILE_SIZE, local_size_z = 1) in;
void main()
{
	const ivec2 ImageSize = imageSize(cs_InputTexture);
	
	const int x = int(gl_GlobalInvocationID.x);
	const int y = int(gl_GlobalInvocationID.y);
	
	const float sigmaColor = 30.0; // in mm
	const float sigmaSpace = 4.5; //in pixels

	const float sigmaColor2_inv = 1.0 / (sigmaColor * sigmaColor);
	const float sigmaSpace2_inv = 1.0 / (sigmaSpace * sigmaSpace);

	const int R = 6; // int(sigmaSpace * 1,5)
	const int D = R * 2 + 1;

	const int Depth = int(imageLoad(cs_InputTexture, ivec2(x, y)).r);
		
	const int tx = min(x - D / 2 + D, ImageSize.x - 1);
	const int ty = min(y - D / 2 + D, ImageSize.y - 1);

	float Normalization = 1.0;
	float Result = 0.0;

	for (int cy = -R; cy < R; ++ cy)
	{
		for (int cx = -R; cx < R; ++ cx)
		{
			const ivec2 SamplePos = ivec2(cx + x, cy + y);
			const int SampleDepth = int(imageLoad(cs_InputTexture, SamplePos).r);
			
			const float Closeness = 1.0;//exp(-((SampleDepth - Depth) * (SampleDepth - Depth) * sigmaColor2_inv));
			
			Normalization += Closeness;
			Result += Closeness * SampleDepth;
		}
	}

	const int result = int(Result / Normalization);
	imageStore(cs_OutputTexture, ivec2(x, y), ivec4(Depth, 0, 0, 0));
	//imageStore(cs_OutputTexture, ivec2(x, y), ivec4(max(0, min(result, 32767))));
}

#endif // __INCLUDE_CS_KINECT_BILATERAL_FILTER_GLSL__