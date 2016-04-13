
#ifndef __INCLUDE_CS_GAUSSIAN_BLUR_RGBA16F_GLSL__
#define __INCLUDE_CS_GAUSSIAN_BLUR_RGBA16F_GLSL__

// -----------------------------------------------------------------------------
// Defines
// -----------------------------------------------------------------------------
#define TILE_SIZE 8

// -----------------------------------------------------------------------------
// Input from engine
// -----------------------------------------------------------------------------
layout(std430, binding = 0) readonly buffer UGaussianSettings
{
    uvec4 cs_ConstantData0;
    float m_Weights[7];
};

layout (binding = 0, rgba16f) readonly  uniform image2D cs_InputTexture;
layout (binding = 1, rgba16f) writeonly uniform image2D cs_OutputTexture;

// -------------------------------------------------------------------------------------
// Functions
// -------------------------------------------------------------------------------------
layout (local_size_x = TILE_SIZE, local_size_y = TILE_SIZE, local_size_z = 1) in;
void main()
{
    uvec2 PixelCoord;
    vec4  BlurredTexture;

    // -----------------------------------------------------------------------------
    // Settings
    // -----------------------------------------------------------------------------
    uvec2 cs_Direction     = cs_ConstantData0.xy;
    uvec2 cs_MaxPixelCoord = cs_ConstantData0.zw;

    // -----------------------------------------------------------------------------
    // Is pixel inside output texture
    // -----------------------------------------------------------------------------
    PixelCoord = gl_GlobalInvocationID.xy;

    bvec2 IsInside;

    IsInside.x = PixelCoord.x < cs_MaxPixelCoord.x;
    IsInside.y = PixelCoord.y < cs_MaxPixelCoord.y;
    
    if (all(IsInside))
    {
        BlurredTexture = vec4(0.0f);

        // -----------------------------------------------------------------------------
        // Do the gaussian blur with the given kernel
        // -----------------------------------------------------------------------------
        BlurredTexture += imageLoad(cs_InputTexture, ivec2(PixelCoord) - ivec2(6) * ivec2(cs_Direction)) * m_Weights[0];
        BlurredTexture += imageLoad(cs_InputTexture, ivec2(PixelCoord) - ivec2(5) * ivec2(cs_Direction)) * m_Weights[1];
        BlurredTexture += imageLoad(cs_InputTexture, ivec2(PixelCoord) - ivec2(4) * ivec2(cs_Direction)) * m_Weights[2];
        BlurredTexture += imageLoad(cs_InputTexture, ivec2(PixelCoord) - ivec2(3) * ivec2(cs_Direction)) * m_Weights[3];
        BlurredTexture += imageLoad(cs_InputTexture, ivec2(PixelCoord) - ivec2(2) * ivec2(cs_Direction)) * m_Weights[4];
        
        BlurredTexture += imageLoad(cs_InputTexture, ivec2(PixelCoord) - ivec2(cs_Direction)) * m_Weights[5];
        
        BlurredTexture += imageLoad(cs_InputTexture, ivec2(PixelCoord)) * m_Weights[6];
        
        BlurredTexture += imageLoad(cs_InputTexture, ivec2(PixelCoord) + ivec2(cs_Direction)) * m_Weights[5];
        
        BlurredTexture += imageLoad(cs_InputTexture, ivec2(PixelCoord) + ivec2(2) * ivec2(cs_Direction)) * m_Weights[4];
        BlurredTexture += imageLoad(cs_InputTexture, ivec2(PixelCoord) + ivec2(3) * ivec2(cs_Direction)) * m_Weights[3];
        BlurredTexture += imageLoad(cs_InputTexture, ivec2(PixelCoord) + ivec2(4) * ivec2(cs_Direction)) * m_Weights[2];
        BlurredTexture += imageLoad(cs_InputTexture, ivec2(PixelCoord) + ivec2(5) * ivec2(cs_Direction)) * m_Weights[1];
        BlurredTexture += imageLoad(cs_InputTexture, ivec2(PixelCoord) + ivec2(6) * ivec2(cs_Direction)) * m_Weights[0];

        // -----------------------------------------------------------------------------
        // Return final color
        // -----------------------------------------------------------------------------
        imageStore(cs_OutputTexture, ivec2(PixelCoord), BlurredTexture);   
    }
}

#endif // __INCLUDE_CS_GAUSSIAN_BLUR_RGBA16F_GLSL__