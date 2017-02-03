#ifndef __INCLUDE_CS_LIGHT_AREALIGHT_BLU_GLSL__
#define __INCLUDE_CS_LIGHT_AREALIGHT_BLU_GLSL__

// -----------------------------------------------------------------------------
// Input from engine
// -----------------------------------------------------------------------------
layout(std430, binding = 0) readonly buffer UBlurProperties 
{ 
    uvec4 cs_ConstantData0; 
}; 

layout(std430, binding = 1) readonly buffer UFilterProperties
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

    vec2 UV  = vec2(PixelCoord) * cs_InverseSizeAndOffset.xy;
    vec2 UV2 = (UV - 0.125f) * (1 + 0.334f);

    if (UV2.x >= 0.0f && UV2.y >= 0.0f && UV2.x <= 1.0f && UV2.y <= 1.0f)
    {
        vec4 BlurredTexture = vec4(0.0f); 
     
        int Area = int(cs_LOD * 4); 
        
        float Count = 0.0f;

        for (int Index = -Area; Index <= Area; ++ Index) 
        { 
            vec2 TexCoord = UV + (vec2(Index) * vec2(cs_Direction) / vec2(cs_MaxPixelCoord));

            BlurredTexture += texture(in_Texture, TexCoord);
            
            Count = Count + 1.0f;
        } 

        Output = BlurredTexture / (Area + Area + 1); 
    }
    else
    {
        Output = texture(in_Texture, UV);
    }
    
    
    imageStore(out_FilteredTexture, ivec2(PixelCoord.x, PixelCoord.y), Output);
}

#endif // __INCLUDE_CS_LIGHT_AREALIGHT_BLU_GLSL__