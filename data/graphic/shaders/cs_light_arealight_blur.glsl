#ifndef __INCLUDE_CS_LIGHT_AREALIGHT_FILTER_GLSL__
#define __INCLUDE_CS_LIGHT_AREALIGHT_FILTER_GLSL__

// -----------------------------------------------------------------------------
// Input from engine
// -----------------------------------------------------------------------------
layout(std430, binding = 0) readonly buffer UFilterProperties
{
    vec4 cs_InverseSizeAndOffset;
    uint cs_LOD;
};

layout (binding = 0) uniform sampler2D in_Texture;

// -----------------------------------------------------------------------------
// Output
// -----------------------------------------------------------------------------
layout (binding = 0, rgba8) uniform image2D out_FilteredTexture;

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
    // Initialization
    // -----------------------------------------------------------------------------    
    PixelCoordX = gl_GlobalInvocationID.x;
    PixelCoordY = gl_GlobalInvocationID.y;
    Output      = vec4(0.0f);

    uvec2 PixelCoord = gl_GlobalInvocationID.xy;
    
    vec2 UV  = vec2(PixelCoord) * cs_InverseSizeAndOffset.xy;
    vec2 UV2 = (UV - 0.125f) * (1 + 0.334f);

    vec2 BorderUV = vec2(0.0f);
    
    float Distance = 0.0f;
    
    int Area = 0;
    
    if (!(UV2.x >= 0.0f && UV2.y >= 0.0f && UV2.x <= 1.0f && UV2.y <= 1.0f))
    {
        if (UV.x <= cs_InverseSizeAndOffset.z)
        {
            // Left
            BorderUV = vec2(cs_InverseSizeAndOffset.z, UV.y);
        }
        else if (UV.x >= 1.0f - cs_InverseSizeAndOffset.z)
        {
            // Right
            BorderUV = vec2(1.0f - cs_InverseSizeAndOffset.z, UV.y);
        }
        else if (UV.y <= cs_InverseSizeAndOffset.w)
        {
            // Lower
            BorderUV = vec2(UV.x, cs_InverseSizeAndOffset.w);
        }
        else if (UV.y >= 1.0f - cs_InverseSizeAndOffset.w)
        {
            // Upper
            BorderUV = vec2(UV.x, 1.0f - cs_InverseSizeAndOffset.w);
        }
        
        if (UV.x <= cs_InverseSizeAndOffset.z && UV.y <= cs_InverseSizeAndOffset.w)
        {
            // Lower Left
            BorderUV = vec2(cs_InverseSizeAndOffset.z, cs_InverseSizeAndOffset.w);
        }
        else if (UV.x <= cs_InverseSizeAndOffset.z && UV.y >= 1.0f - cs_InverseSizeAndOffset.w)
        {
            // Upper Left
            BorderUV = vec2(cs_InverseSizeAndOffset.z, 1.0f - cs_InverseSizeAndOffset.w);
        }
        else if (UV.x >= 1.0f - cs_InverseSizeAndOffset.z && UV.y <= cs_InverseSizeAndOffset.w)
        {
            // Lower Right
            BorderUV = vec2(1.0f - cs_InverseSizeAndOffset.z, cs_InverseSizeAndOffset.w);
        }
        else if (UV.x >= 1.0f - cs_InverseSizeAndOffset.z && UV.y >= 1.0f - cs_InverseSizeAndOffset.w)
        {
            // Upper Right
            BorderUV = vec2(1.0f - cs_InverseSizeAndOffset.z, 1.0f - cs_InverseSizeAndOffset.w);
        }
		
		vec2 DistanceVec = UV - BorderUV;
    
		Distance = sqrt(DistanceVec.x * DistanceVec.x + DistanceVec.y * DistanceVec.y) / cs_InverseSizeAndOffset.z;

		Area = int(max(Distance, 0.0f) * 8.0f);
	
		vec4 BlurredTexture = vec4(0.0f);

		for (int Y = -Area; Y <= Area; ++ Y)
		{
			for (int X = -Area; X <= Area; ++ X)
			{
				vec2 ReadUV = UV + ivec2(X, Y) * cs_InverseSizeAndOffset.xy;
				
				vec4 Color = texture(in_Texture, ReadUV);
			
				BlurredTexture += Color;
			}
		}
		
		Output = BlurredTexture / ((Area + 1 + Area));
			        
	    imageStore(out_FilteredTexture, ivec2(PixelCoordX, PixelCoordY), Output);
    }
}

#endif // __INCLUDE_CS_LIGHT_AREALIGHT_FILTER_GLSL__