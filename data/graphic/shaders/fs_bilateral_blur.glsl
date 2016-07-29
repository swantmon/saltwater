
#ifndef __INCLUDE_FS_BILATERAL_BLUR_GLSL__
#define __INCLUDE_FS_BILATERAL_BLUR_GLSL__

#include "functions.glsl"

// -----------------------------------------------------------------------------
// Input from engine
// -----------------------------------------------------------------------------
layout(row_major, std140, binding = 0) uniform UB0
{
    vec2 ps_Direction;
};

layout(binding = 0) uniform sampler2D ps_InputTexture;

// -----------------------------------------------------------------------------
// Input
// -----------------------------------------------------------------------------
layout(location = 2) in vec2 in_UV;

// -----------------------------------------------------------------------------
// Output
// -----------------------------------------------------------------------------
layout(location = 0) out float out_Output;

// -----------------------------------------------------------------------------
// Constants
// -----------------------------------------------------------------------------
const float BlurScale        = 0.1f;
const float BlurDepthFalloff = 100.0f;

// -----------------------------------------------------------------------------
// Function
// -----------------------------------------------------------------------------
void main(void)
{
    float CenterFragment  = texture(ps_InputTexture, in_UV).x;
    float CurrentFragment = 0.0f;
    float FinalFragment   = 0.0f;
    
    float FactorR1  = 0.0f;
    float FactorR2  = 0.0f;
    float Factor    = 0.0f;
    float WeightSum = 0.0f;
    
    // -----------------------------------------------------------------------------
    // Calulate blur size depending on distance
    // -----------------------------------------------------------------------------
    float BlurSize = 12.0f; //max((1.0f - ConvertToLinearDepth(CenterFragment, 0.01f, 4096.0f)) * 64.0f, 1.0f);
    
    // -----------------------------------------------------------------------------
    // Do the bilateral blur with the given kernel
    // -----------------------------------------------------------------------------
    for (float CurrentPixelIndex = -BlurSize; CurrentPixelIndex <= BlurSize; CurrentPixelIndex += 1.0f)
    {
        CurrentFragment = texture(ps_InputTexture, in_UV + vec2(CurrentPixelIndex) * ps_Direction).x;
        
        FactorR1        = CurrentPixelIndex * BlurScale;
        FactorR2        = (CurrentFragment - CenterFragment) * BlurDepthFalloff;
        Factor          = exp(-FactorR1 * FactorR1) * exp(-FactorR2 * FactorR2);
        FinalFragment  += CurrentFragment * Factor;
        WeightSum      += Factor;
    }
    
    out_Output = FinalFragment / WeightSum;
}

#endif // __INCLUDE_FS_BILATERAL_BLUR_GLSL__