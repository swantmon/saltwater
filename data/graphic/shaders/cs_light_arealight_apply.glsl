
#ifndef __INCLUDE_CS_LIGHT_AREALIGHT_APPLY_GLSL__
#define __INCLUDE_CS_LIGHT_AREALIGHT_APPLY_GLSL__

// -----------------------------------------------------------------------------
// Output
// -----------------------------------------------------------------------------
layout(std430, binding = 0) readonly buffer UApplyProperties
{
    vec4 cs_InverseSizeAndOffset;
    uint cs_LOD;
};

layout (binding = 0) uniform sampler2D in_OuterTexture;
layout (binding = 1) uniform sampler2D in_InnerTexture;
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
    int InnerRectX1 = int((1.0f / cs_InverseSizeAndOffset.x) * cs_InverseSizeAndOffset.z);
    int InnerRectX2 = int((1.0f / cs_InverseSizeAndOffset.x) * (1.0f - cs_InverseSizeAndOffset.z));
    int InnerRectY1 = int((1.0f / cs_InverseSizeAndOffset.y) * cs_InverseSizeAndOffset.w);
    int InnerRectY2 = int((1.0f / cs_InverseSizeAndOffset.y) * (1.0f - cs_InverseSizeAndOffset.w));

    vec2 UV =  vec2(PixelCoordX, PixelCoordY) * cs_InverseSizeAndOffset.xy;

    if (PixelCoordX > InnerRectX1 && PixelCoordX < InnerRectX2 && PixelCoordY > InnerRectY1 && PixelCoordY < InnerRectY2)
    {
        UV = UV * (1.0f + cs_InverseSizeAndOffset.zw * 2.0f) - cs_InverseSizeAndOffset.zw;

        Output = texture(in_InnerTexture, UV);
    }
    else
    {
         Output = texture(in_OuterTexture, UV);
    }

    imageStore(out_FilteredTexture, ivec2(PixelCoordX, PixelCoordY), Output);
}

#endif // __INCLUDE_CS_LIGHT_AREALIGHT_APPLY_GLSL__