
#ifndef __INCLUDE_CS_LIGHT_AREALIGHT_APPLY_GLSL__
#define __INCLUDE_CS_LIGHT_AREALIGHT_APPLY_GLSL__

// -----------------------------------------------------------------------------
// Output
// -----------------------------------------------------------------------------
layout(std430, binding = 0) readonly buffer UApplyProperties
{
    uint cs_LOD;
};

layout (binding = 0) uniform sampler2D in_Texture;
layout (binding = 1) uniform sampler2D in_OrigTexture;
layout (binding = 0, rgba16f) writeonly uniform image2D out_FilteredTexture;

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
    
    LOD = 0;
    
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
    
    vec4 OuterPixel = texture(in_Texture, UV);
    
    UV = UV * (1.0f + 0.125f * 2.0f) - 0.125f;

    vec4 InnerPixel = texture(in_OrigTexture, UV);
    
    float Mixing = 0.0f;
    
    
    if (PixelCoordX > InnerRectX1 && PixelCoordX < InnerRectX2 && PixelCoordY > InnerRectY1 && PixelCoordY < InnerRectY2)
    {
        Mixing = 0.0f;
    }
    else
    {
        Mixing = sqrt((PixelCoordX - 1024.0f) * (PixelCoordX - 1024.0f) + (PixelCoordY - 1024.0f) * (PixelCoordY - 1024.0f));
        
        Mixing = Mixing / 1024.0f;
        
        Mixing *= 1.35f;
    }
    
    Output = mix(InnerPixel, OuterPixel, clamp(Mixing, 0.0f, 1.0f));

    imageStore(out_FilteredTexture, ivec2(PixelCoordX, PixelCoordY), Output);
}

#endif // __INCLUDE_CS_LIGHT_AREALIGHT_APPLY_GLSL__