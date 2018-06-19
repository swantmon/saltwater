#ifndef __INCLUDE_CS_LIGHT_AREALIGHT_FILTER_GLSL__
#define __INCLUDE_CS_LIGHT_AREALIGHT_FILTER_GLSL__

// -----------------------------------------------------------------------------
// Input from engine
// -----------------------------------------------------------------------------
layout(std430, binding = 0) readonly buffer UBlurProperties 
{ 
    uvec2 cs_Direction;
    uint  cs_LOD;
}; 

layout(std430, binding = 1) readonly buffer UFilterProperties
{
	vec4 cs_InverseSizeAndOffset;
};

// -----------------------------------------------------------------------------
// Output
// -----------------------------------------------------------------------------
layout (binding = 0) uniform sampler2D in_Texture0;
layout (binding = 1) uniform sampler2D in_Texture1;

layout (binding = 0, rgba8) writeonly uniform image2D out_Texture;

// -------------------------------------------------------------------------------------
// Layout
// -------------------------------------------------------------------------------------
layout (local_size_x = TILE_SIZE, local_size_y = TILE_SIZE, local_size_z = 1) in;

// -------------------------------------------------------------------------------------
// Functions
// -------------------------------------------------------------------------------------
void Filter()
{
	uint PixelCoordX;
    uint PixelCoordY;
	vec4 Output;

    // -------------------------------------------------------------------------------------
	// Initialization
    // -------------------------------------------------------------------------------------
    PixelCoordX = gl_GlobalInvocationID.x;
    PixelCoordY = gl_GlobalInvocationID.y;
	
    // -------------------------------------------------------------------------------------
    // Define inner and outer area
    // -------------------------------------------------------------------------------------
    vec2 UV =  vec2(PixelCoordX, PixelCoordY) * cs_InverseSizeAndOffset.xy;

	UV = (UV - cs_InverseSizeAndOffset.zw) * (1.0f + 0.334f);

    // -------------------------------------------------------------------------------------
    // Inner part is the texture; Outer part is potencial background
    // -------------------------------------------------------------------------------------
    if (UV.x >= 0.0f && UV.y >= 0.0f && UV.x <= 1.0f && UV.y <= 1.0f)
    {
	   vec4 Output = texture(in_Texture0, UV);

       imageStore(out_Texture, ivec2(PixelCoordX, PixelCoordY), Output);
    }
    else
    {
        imageStore(out_Texture, ivec2(PixelCoordX, PixelCoordY), vec4(0.25f));  
    }
}

// -------------------------------------------------------------------------------------

void BlurBackground()
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
    
    // -------------------------------------------------------------------------------------
    // Define inner and outer area
    // -------------------------------------------------------------------------------------
    vec2 UV        = vec2(PixelCoord) * cs_InverseSizeAndOffset.xy;
    vec2 ClampedUV = (UV - cs_InverseSizeAndOffset.zw) * (1.0f + 0.334f);

    vec2 BorderUV = vec2(0.0f);
    
    float Distance = 0.0f;
    
    int Area = 0;
    
    // -------------------------------------------------------------------------------------
    // Outer area is a distnace based blur to the inner BorderUV
    // Inner area is simply the given texture without blur
    // -------------------------------------------------------------------------------------
    if (!(ClampedUV.x >= 0.0f && ClampedUV.y >= 0.0f && ClampedUV.x <= 1.0f && ClampedUV.y <= 1.0f))
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

        Area = int(max(Distance, 0.0f) * 64.0f);
    
        vec4 BlurredTexture = vec4(0.0f);
        
        float Count = 0.0f;

        for (int Y = -Area; Y <= Area; ++ Y)
        {
            for (int X = -Area; X <= Area; ++ X)
            {
                vec2 ReadUV = UV + vec2(X, Y) * cs_InverseSizeAndOffset.xy;
                
                vec4 Color = texture(in_Texture0, ReadUV);
            
                BlurredTexture += Color;
                
                if (Color.a > 0.0f) Count += Color.a;
            }
        }
        
        Output = BlurredTexture / float(Count);
                        
        imageStore(out_Texture, ivec2(PixelCoordX, PixelCoordY), Output);
    }
}

// -----------------------------------------------------------------------------

void BlurForeground()
{
    // -----------------------------------------------------------------------------
    // Initialization
    // -----------------------------------------------------------------------------    
    vec4 Output = vec4(0.0f);

    uvec2 PixelCoord = gl_GlobalInvocationID.xy;

    // -------------------------------------------------------------------------------------
    // Deinfe inner and outer part
    // -------------------------------------------------------------------------------------
    vec2 UV        = vec2(PixelCoord) * cs_InverseSizeAndOffset.xy;
    vec2 ClampedUV = (UV - cs_InverseSizeAndOffset.zw) * (1.0f + 0.334f);

    // -------------------------------------------------------------------------------------
    // Inner part is a bluerred foreground (splitted horizontal and vertical blur)
    // Outer part is pre blurred texture
    // -------------------------------------------------------------------------------------
    if (ClampedUV.x >= -0.8f && ClampedUV.y >= -0.8f && ClampedUV.x <= 1.2f && ClampedUV.y <= 1.2f)
    {
        vec4 BlurredTexture = vec4(0.0f); 
     
        int Area = int(cs_LOD) * int(2.0f); 

        for (int Index = -Area; Index <= Area; ++ Index) 
        { 
            vec2 TexCoord = UV + (vec2(Index) * vec2(cs_Direction) * cs_InverseSizeAndOffset.xy);

            BlurredTexture += textureLod(in_Texture0, TexCoord, float(cs_LOD));
        } 

        Output = BlurredTexture / float((Area + Area + 1)); 
    }
    else
    {
        Output = texture(in_Texture0, UV);
    }
    
    
    imageStore(out_Texture, ivec2(PixelCoord.x, PixelCoord.y), Output);
}

// -------------------------------------------------------------------------------------

void Combine()
{
    uint PixelCoordX;
    uint PixelCoordY;
    vec4 Output;
    
    // -------------------------------------------------------------------------------------
    // Initialization
    // -------------------------------------------------------------------------------------
    PixelCoordX = gl_GlobalInvocationID.x;
    PixelCoordY = gl_GlobalInvocationID.y;
    Output      = vec4(0.0f);

    // -------------------------------------------------------------------------------------
    // Define inner part
    // -------------------------------------------------------------------------------------
    vec2 UV =  vec2(PixelCoordX, PixelCoordY) * cs_InverseSizeAndOffset.xy;
    
    vec2 ClampedUV = (UV - cs_InverseSizeAndOffset.zw) * (1.0f + 0.334f);

    // -------------------------------------------------------------------------------------
    // Combine inner and outer part
    // -------------------------------------------------------------------------------------
    if (ClampedUV.x >= 0.0f && ClampedUV.y >= 0.0f && ClampedUV.x <= 1.0f && ClampedUV.y <= 1.0f)
    {
        Output = texture(in_Texture1, ClampedUV);
    }
    else
    {
        Output = texture(in_Texture0, UV);
    }

    imageStore(out_Texture, ivec2(PixelCoordX, PixelCoordY), Output);
}


#endif // __INCLUDE_CS_LIGHT_AREALIGHT_FILTER_GLSL__