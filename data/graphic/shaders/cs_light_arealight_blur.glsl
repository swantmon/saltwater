
#ifndef __INCLUDE_CS_LIGHT_AREALIGHT_FILTER_GLSL__
#define __INCLUDE_CS_LIGHT_AREALIGHT_FILTER_GLSL__

// -----------------------------------------------------------------------------
// Input from engine
// -----------------------------------------------------------------------------
layout(std430, binding = 0) readonly buffer UFilterProperties
{
    uint cs_LOD;
};

layout(std430, binding = 1) readonly buffer UGaussianProperties
{
    uvec4 cs_ConstantData0;
    float m_Weights[7];
};

// -----------------------------------------------------------------------------
// Output
// -----------------------------------------------------------------------------
layout (binding = 0, rgba16f) uniform image2D out_FilteredTexture;

// -------------------------------------------------------------------------------------
// Functions
// -------------------------------------------------------------------------------------
layout (local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main()
{
	uint PixelCoordX;
    uint PixelCoordY;
	vec4 Output;
	int  LOD;

	// -----------------------------------------------------------------------------
    // Settings
    // -----------------------------------------------------------------------------
    uvec2 cs_Direction     = cs_ConstantData0.xy;
    uvec2 cs_MaxPixelCoord = cs_ConstantData0.zw;
	
	// Initialization
    PixelCoordX = gl_GlobalInvocationID.x;
    PixelCoordY = gl_GlobalInvocationID.y;
	Output      = vec4(0.0f);

	// Define inner part
	int InnerRectX1 = int(2048.0f * 0.125f);
	int InnerRectX2 = int(2048.0f * (1.0f - 0.125f));
	int InnerRectY1 = int(2048.0f * 0.125f);
	int InnerRectY2 = int(2048.0f * (1.0f - 0.125f));

	uvec2 PixelCoord = gl_GlobalInvocationID.xy;
	

	if (PixelCoordX > InnerRectX1 && PixelCoordX < InnerRectX2 && PixelCoordY > InnerRectY1 && PixelCoordY < InnerRectY2)
	{
		LOD	= 1;
		
		Output = imageLoad(out_FilteredTexture, ivec2(PixelCoordX, PixelCoordY));
	}
    else
	{
		vec4 BlurredTexture = vec4(0.0f);
		
		int Area = 7 - 1;
		
		for (int Index = -Area; Index <= Area; ++ Index)
		{
			uint I = Area - abs(Index);
			
			BlurredTexture += imageLoad(out_FilteredTexture, ivec2(PixelCoord) + ivec2(Index) * ivec2(cs_Direction)) * m_Weights[I];
		}

		Output = BlurredTexture;
	}

    imageStore(out_FilteredTexture, ivec2(PixelCoordX, PixelCoordY), Output);
}

#endif // __INCLUDE_CS_LIGHT_AREALIGHT_FILTER_GLSL__