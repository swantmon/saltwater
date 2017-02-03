#ifndef __INCLUDE_CS_LIGHT_AREALIGHT_BLU_GLSL__
#define __INCLUDE_CS_LIGHT_AREALIGHT_BLU_GLSL__

// -----------------------------------------------------------------------------
// Input from engine
// -----------------------------------------------------------------------------
layout(std430, binding = 0) readonly buffer UBlurProperties 
{ 
    uvec4 cs_ConstantData0; 
}; 

// -----------------------------------------------------------------------------
// Output
// -----------------------------------------------------------------------------
layout (binding = 0, rgba8) uniform image2D out_FilteredTexture;

// -------------------------------------------------------------------------------------
// Functions
// -------------------------------------------------------------------------------------
layout (local_size_x = 8, local_size_y = 8, local_size_z = 1) in;
void main()
{
    uvec2 cs_Direction     = cs_ConstantData0.xy; 
    uvec2 cs_MaxPixelCoord = cs_ConstantData0.zw; 

    // -----------------------------------------------------------------------------
    // Initialization
    // -----------------------------------------------------------------------------    
    vec4 Output = vec4(0.0f);

    uvec2 PixelCoord = gl_GlobalInvocationID.xy;
    
    vec4 BlurredTexture = vec4(0.0f); 
     
    int Area = 12 - 1; 
    
    float Count = 0.0f;
     
    for (int Index = -Area; Index <= Area; ++ Index) 
    { 
        ivec2 TexCoord = ivec2(PixelCoord) + ivec2(Index) * ivec2(cs_Direction);
        
        if (TexCoord.x >= 0 && TexCoord.y >= 0 && TexCoord.x <= cs_MaxPixelCoord.x && TexCoord.y <= cs_MaxPixelCoord.y)
        {
            BlurredTexture += imageLoad(out_FilteredTexture, TexCoord);
            
            Count = Count + 1.0f;
        }
    } 
 
    Output = BlurredTexture / Count; 
 
    imageStore(out_FilteredTexture, ivec2(PixelCoord.x, PixelCoord.y), Output); 
}

#endif // __INCLUDE_CS_LIGHT_AREALIGHT_BLU_GLSL__