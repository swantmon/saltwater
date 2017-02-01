#ifndef __INCLUDE_CS_LIGHT_AREALIGHT_FILTER_GLSL__
#define __INCLUDE_CS_LIGHT_AREALIGHT_FILTER_GLSL__

// -----------------------------------------------------------------------------
// Output
// -----------------------------------------------------------------------------
layout (binding = 0) uniform sampler2D in_Texture;
layout (binding = 1, rgba16f) writeonly uniform image2D out_FilteredTexture;

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
	
	// Initialization
    PixelCoordX = gl_GlobalInvocationID.x;
    PixelCoordY = gl_GlobalInvocationID.y;
	Output      = vec4(0.0f);

	// Define inner part
	int InnerRectX1 = int(2048.0f * 0.125f);
	int InnerRectX2 = int(2048.0f * (1.0f - 0.125f));
	int InnerRectY1 = int(2048.0f * 0.125f);
	int InnerRectY2 = int(2048.0f * (1.0f - 0.125f));
	
	
	vec2 UV =  vec2(PixelCoordX, PixelCoordY) * vec2(1.0f / 2048.0f);

	UV = UV * (1.0f + 0.125f * 2) - 0.125f;

	if (PixelCoordX > InnerRectX1 && PixelCoordX < InnerRectX2 && PixelCoordY > InnerRectY1 && PixelCoordY < InnerRectY2)
	{
		LOD	= 1;
		
		// Inner
		vec4 BlurredTexture = vec4(0.0f);
		for (int Y = -LOD; Y <= LOD; ++Y)
		{
			for (int X = -LOD; X <= LOD; ++X)
			{
				BlurredTexture += texture(in_Texture, UV + vec2(X, Y) * vec2(1.0f / 2048.0f));
			}
		}
		BlurredTexture /= ((LOD + LOD + 1) * (LOD + LOD + 1));

		Output += BlurredTexture;
	}
    else
	{
		LOD	= 22;
		
		vec4 BlurredTexture = vec4(0.0f);
		for (int Y = -LOD; Y <= LOD; ++Y)
		{
			for (int X = -LOD; X <= LOD; ++X)
			{
				BlurredTexture += texture(in_Texture, UV + vec2(X, Y) * vec2(1.0f / 2048.0f));
			}
		}
		BlurredTexture /= ((LOD + LOD + 1) * (LOD + LOD + 1));

		Output += BlurredTexture;
	}

    imageStore(out_FilteredTexture, ivec2(PixelCoordX, PixelCoordY), Output);
}

#endif // __INCLUDE_CS_LIGHT_AREALIGHT_FILTER_GLSL__