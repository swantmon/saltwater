
#ifndef __INCLUDE_FS_DOF_NEAR_BLUR_GLSL__
#define __INCLUDE_FS_DOF_NEAR_BLUR_GLSL__

// -----------------------------------------------------------------------------
// Input from engine
// -----------------------------------------------------------------------------
layout(binding = 0) uniform sampler2D ps_Near;

// -----------------------------------------------------------------------------
// Input
// -----------------------------------------------------------------------------
struct SPixelCoords
{
    vec4 m_TexCoords;
};

layout(location = 0) in SPixelCoords in_PixelCoords;

// -----------------------------------------------------------------------------
// Output
// -----------------------------------------------------------------------------
layout(location = 0) out vec4 out_Output;

// -----------------------------------------------------------------------------
// Function
// More info at: http://http.developer.nvidia.com/GPUGems3/gpugems3_ch28.html
// -----------------------------------------------------------------------------
void main(void)
{
    // -----------------------------------------------------------------------------
    // Initialize variables
    // -----------------------------------------------------------------------------
    vec4  Color;
    
    // -----------------------------------------------------------------------------
    // Blur color
    // -----------------------------------------------------------------------------
    Color = vec4(0.0f);
    
    Color += texture(ps_Near, in_PixelCoords.m_TexCoords.xz);
    Color += texture(ps_Near, in_PixelCoords.m_TexCoords.yz);
    Color += texture(ps_Near, in_PixelCoords.m_TexCoords.xw);
    Color += texture(ps_Near, in_PixelCoords.m_TexCoords.yw);
    
    Color /= 4.0f;

    // -----------------------------------------------------------------------------
    // Return final color
    // -----------------------------------------------------------------------------
    out_Output = vec4(Color);
}

#endif // __INCLUDE_FS_DOF_NEAR_BLUR_GLSL__