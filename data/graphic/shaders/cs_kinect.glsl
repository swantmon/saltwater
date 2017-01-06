
#ifndef __INCLUDE_CS_VOXEL_GLSL__
#define __INCLUDE_CS_VOXEL_GLSL__

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
	ivec2 ImageSize = imageSize(cs_InputTexture);
	
	const uint x = gl_GlobalInvocationID.x;
	const uint y = gl_GlobalInvocationID.y;
	
	if (x >= ImageSize.x || y >= ImageSize.y)
		return;
	
	const float sigmaColor = 30.0; // in mm
	const float sigmaSpace = 4.5; //in pixels

	const float sigmaColor2_inv = 1.0 / (sigmaColor * sigmaColor);
	const float sigmaSpace2_inv = 1.0 / (sigmaSpace * sigmaSpace);

	const uint R = 6; // int(sigmaSpace * 1,5)
	const uint D = R * 2 + 1;

	uint Depth = imageLoad(cs_InputTexture, ivec2(y, x)).r;
		
	const uint tx = min(x - D / 2 + D, ImageSize.x - 1);
	const uint ty = min(y - D / 2 + D, ImageSize.y - 1);

	float sum1 = 0.0;
	float sum2 = 0.0;

	for (uint cy = max(y - D / 2, 0); cy < ty; ++cy)
	{
		for (uint cx = max(x - D / 2, 0); cx < tx; ++cx)
		{
			const uint DepthTmp = imageLoad(cs_InputTexture, ivec2(cy, cx)).r;

			const float space2 = (x - cx) * (x - cx) + (y - cy) * (y - cy);
			const float color2 = (Depth - DepthTmp) * (Depth - DepthTmp);

			const float weight = exp(-(space2 * sigmaSpace2_inv + color2 * sigmaColor2_inv));

			sum1 += DepthTmp * weight;
			sum2 += weight;
		}
	}

	int result = int(sum1 / sum2);
	
    imageStore(cs_OutputTexture, ivec2(y, x), ivec4(max(0, min(result, 32767))));
}

#endif // __INCLUDE_CS_VOXEL_GLSL__