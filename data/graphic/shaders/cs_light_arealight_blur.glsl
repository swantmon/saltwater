
#ifndef __INCLUDE_CS_LIGHT_AREALIGHT_FILTER_GLSL__
#define __INCLUDE_CS_LIGHT_AREALIGHT_FILTER_GLSL__

// -----------------------------------------------------------------------------
// Input from engine
// -----------------------------------------------------------------------------
layout(std430, binding = 0) readonly buffer UGaussianProperties
{
    uvec4 cs_ConstantData0;
    float m_Weights[7];
};

layout(std430, binding = 1) readonly buffer UFilterProperties
{
	vec4 cs_InverseSizeAndOffset;
    uint cs_LOD;
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

	uvec2 PixelCoord = gl_GlobalInvocationID.xy;
	
	vec4 BlurredTexture = vec4(0.0f);
	
	int Area = 256 - 1;
	
	for (int Index = -Area; Index <= Area; ++ Index)
	{
		ivec2 ReadCoord = ivec2(PixelCoord) + ivec2(Index) * ivec2(cs_Direction);
		
		ReadCoord.x = ReadCoord.x < 0 ? -ReadCoord.x : ReadCoord.x;
		ReadCoord.y = ReadCoord.y < 0 ? -ReadCoord.y : ReadCoord.y;
		
		ReadCoord.x = ReadCoord.x > int(cs_MaxPixelCoord.x) ? int(cs_MaxPixelCoord.x) + int(cs_MaxPixelCoord.x) - ReadCoord.x : ReadCoord.x;
		ReadCoord.y = ReadCoord.y > int(cs_MaxPixelCoord.y) ? int(cs_MaxPixelCoord.y) + int(cs_MaxPixelCoord.y) - ReadCoord.y : ReadCoord.y;
	
		BlurredTexture += imageLoad(out_FilteredTexture, ReadCoord);
	}
	
	Output = BlurredTexture / (Area + 1 + Area);
	
	
	
	
	
	
/*
	vec4 BlurredTexture = vec4(0.0f);
	
	int Area = 7 - 1;
	
	for (int Index = -Area; Index <= Area; ++ Index)
	{
		ivec2 ReadCoord = ivec2(PixelCoord) + ivec2(Index) * ivec2(cs_Direction);
		
		ReadCoord.x = clamp(ReadCoord.x, 0, int(cs_MaxPixelCoord.x) - 1);
		ReadCoord.y = clamp(ReadCoord.y, 0, int(cs_MaxPixelCoord.y) - 1);
		
		uint I = Area - abs(Index);
	
		BlurredTexture += imageLoad(out_FilteredTexture, ReadCoord) * m_Weights[I];
	}

	Output = BlurredTexture;
*/
		
    imageStore(out_FilteredTexture, ivec2(PixelCoordX, PixelCoordY), Output);
}

#endif // __INCLUDE_CS_LIGHT_AREALIGHT_FILTER_GLSL__