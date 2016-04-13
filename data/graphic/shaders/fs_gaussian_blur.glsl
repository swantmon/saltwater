
#ifndef __INCLUDE_FS_GAUSSIAN_BLUR_GLSL__
#define __INCLUDE_FS_GAUSSIAN_BLUR_GLSL__

// -----------------------------------------------------------------------------
// Input from engine
// -----------------------------------------------------------------------------
layout(binding = 0) uniform UGaussianSettings
{
    vec2  ps_Direction;
    float ps_Weights0;
    float ps_Weights1;
    float ps_Weights2;
    float ps_Weights3;
    float ps_Weights4;
    float ps_Weights5;
    float ps_Weights6;
};

layout(binding = 0) uniform sampler2D ps_InputTexture;

// -----------------------------------------------------------------------------
// Input
// -----------------------------------------------------------------------------
layout(location = 2) in vec2 in_UV;

// -----------------------------------------------------------------------------
// Output
// -----------------------------------------------------------------------------
layout(location = 0) out vec4 out_Output;

// -----------------------------------------------------------------------------
// Function
// -----------------------------------------------------------------------------
void main(void)
{
    // -----------------------------------------------------------------------------
    // Create an empty color
    // -----------------------------------------------------------------------------
    vec4 BlurredTexture = vec4(0.0f);
    
    // -----------------------------------------------------------------------------
    // Do the gaussian blur with the given kernel
    // -----------------------------------------------------------------------------
    BlurredTexture += texture(ps_InputTexture, in_UV - vec2(6.0f) * ps_Direction) * ps_Weights0;
    BlurredTexture += texture(ps_InputTexture, in_UV - vec2(5.0f) * ps_Direction) * ps_Weights1;
    BlurredTexture += texture(ps_InputTexture, in_UV - vec2(4.0f) * ps_Direction) * ps_Weights2;
    BlurredTexture += texture(ps_InputTexture, in_UV - vec2(3.0f) * ps_Direction) * ps_Weights3;
    BlurredTexture += texture(ps_InputTexture, in_UV - vec2(2.0f) * ps_Direction) * ps_Weights4;
    
    BlurredTexture += texture(ps_InputTexture, in_UV - ps_Direction) * ps_Weights5;
    
    BlurredTexture += texture(ps_InputTexture, in_UV) * ps_Weights6;
    
    BlurredTexture += texture(ps_InputTexture, in_UV + ps_Direction) * ps_Weights5;
    
    BlurredTexture += texture(ps_InputTexture, in_UV + vec2(2.0f) * ps_Direction) * ps_Weights4;
    BlurredTexture += texture(ps_InputTexture, in_UV + vec2(3.0f) * ps_Direction) * ps_Weights3;
    BlurredTexture += texture(ps_InputTexture, in_UV + vec2(4.0f) * ps_Direction) * ps_Weights2;
    BlurredTexture += texture(ps_InputTexture, in_UV + vec2(5.0f) * ps_Direction) * ps_Weights1;
    BlurredTexture += texture(ps_InputTexture, in_UV + vec2(6.0f) * ps_Direction) * ps_Weights0;

    // -----------------------------------------------------------------------------
    // Return final color into system back buffer
    // -----------------------------------------------------------------------------
    out_Output = BlurredTexture;
}

#endif // __INCLUDE_FS_GAUSSIAN_BLUR_GLSL__