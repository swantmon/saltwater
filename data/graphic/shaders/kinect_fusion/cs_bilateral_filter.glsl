
#ifndef __INCLUDE_CS_KINECT_BILATERAL_FILTER_GLSL__
#define __INCLUDE_CS_KINECT_BILATERAL_FILTER_GLSL__

// -----------------------------------------------------------------------------
// Constants
// -----------------------------------------------------------------------------

const float g_SigmaColor = 30.0f; // in mm
const float g_SigmaSpace = 4.5f; //in pixels

const float g_SigmaColor2 = g_SigmaColor * g_SigmaColor;
const float g_SigmaSpace2 = g_SigmaSpace * g_SigmaSpace;

const float g_SigmaColor2_inv = 1.0f / g_SigmaColor2;
const float g_SigmaSpace2_inv = 1.0f / g_SigmaSpace2;

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
	const ivec2 ImageSize = imageSize(cs_InputTexture);
	
	const int x = int(gl_GlobalInvocationID.x);
	const int y = int(gl_GlobalInvocationID.y);

	const int R = 6; // int(g_SigmaSpace * 1,5)
	const int D = R * 2 + 1;

	const float Depth = float(imageLoad(cs_InputTexture, ivec2(x, y)).x);

	float Sum1 = 0.0;
	float Sum2 = 0.0;

    float ReferenceDepth = float(imageLoad(cs_InputTexture, ivec2(x, y)).x);

	for (int cx = -R; cx < R; ++ cx)
	{
		for (int cy = -R; cy < R; ++ cy)
		{
			const ivec2 SamplePos = ivec2(x + cx, y + cy);
			const float SampleDepth = float(imageLoad(cs_InputTexture, SamplePos).x);

            float Space2 = cx * cx + cy * cy;
            float Color2 = (ReferenceDepth - SampleDepth) * (ReferenceDepth - SampleDepth);

            const float Weight = exp(-(Space2 * g_SigmaSpace2_inv + Color2 * g_SigmaColor2_inv));

			Sum1 += SampleDepth * Weight;
			Sum2 += Weight;
		}
	}

	const float Result = Sum1 / Sum2;
	
	imageStore(cs_OutputTexture, ivec2(x, y), ivec4(Result));
}

#endif // __INCLUDE_CS_KINECT_BILATERAL_FILTER_GLSL__