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
layout (binding = 0, rgba8) writeonly uniform image2D out_FilteredTexture;

// -------------------------------------------------------------------------------------
// Functions
// -------------------------------------------------------------------------------------
layout (local_size_x = 8, local_size_y = 8, local_size_z = 1) in;
void main()
{
    uint PixelCoordX;
    uint PixelCoordY;
    vec4 Output;
    
    // Initialization
    PixelCoordX = gl_GlobalInvocationID.x;
    PixelCoordY = gl_GlobalInvocationID.y;
    Output      = vec4(0.0f);

    // Define inner part
    vec2 UV =  vec2(PixelCoordX, PixelCoordY) * cs_InverseSizeAndOffset.xy;
    
    vec2 ClampedUV = (UV - 0.125f) * (1 + 0.334f);

    if (ClampedUV.x >= 0.0f && ClampedUV.y >= 0.0f && ClampedUV.x <= 1.0f && ClampedUV.y <= 1.0f)
    {
        Output = texture(in_InnerTexture, ClampedUV);
    }
    else
    {
        Output = texture(in_OuterTexture, UV);
    }

    imageStore(out_FilteredTexture, ivec2(PixelCoordX, PixelCoordY), Output);
}

#endif // __INCLUDE_CS_LIGHT_AREALIGHT_APPLY_GLSL__