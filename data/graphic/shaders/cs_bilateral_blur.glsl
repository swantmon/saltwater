#ifndef __INCLUDE_CS_BILATERAL_BLUR_GLSL__
#define __INCLUDE_CS_BILATERAL_BLUR_GLSL__

// -------------------------------------------------------------------------------------
// Defines
// -------------------------------------------------------------------------------------
#define TILE_SIZE          16
#define SIGMA              10.0
#define BSIGMA             0.1
#define KERNEL_SIZE        9

// -----------------------------------------------------------------------------
// Input from engine
// -----------------------------------------------------------------------------
layout(std140, binding = 0) uniform UBilateralBlurSettingBuffer
{
    ivec4 in_Direction;
};

layout (binding = 0, r32f) readonly  uniform image2D cs_InputImage;
layout (binding = 1, r32f) writeonly uniform image2D cs_OutputImage;

// -------------------------------------------------------------------------------------
// Main
// -------------------------------------------------------------------------------------
float normpdf(in float x, in float sigma)
{
    return 0.39894f * exp(-0.5f * x * x / (sigma * sigma)) / sigma;
}

// -----------------------------------------------------------------------------

float normpdf3(in vec3 v, in float sigma)
{
    return 0.39894f * exp(-0.5f * dot(v, v) / (sigma * sigma)) / sigma;
}

// -----------------------------------------------------------------------------

layout(local_size_x = TILE_SIZE, local_size_y = TILE_SIZE, local_size_z = 1) in;

void main()
{
    uint X = gl_GlobalInvocationID.x;
    uint Y = gl_GlobalInvocationID.y;

    float CenterFragment  = imageLoad(cs_InputImage, ivec2(X, Y)).r;
    float CurrentFragment = 0.0f;
    float FinalFragment   = 0.0f;

    float FactorR1   = 0.0f;
    float FactorR2   = 0.0f;
    float Factor     = 0.0f;
    float WeightSum  = 0.0f;
    
    #define DEBUG
    #ifdef DEBUG
    imageStore(cs_OutputImage, ivec2(X, Y), vec4(CenterFragment));
    return;
    #endif

    // -----------------------------------------------------------------------------
    // Do the bilateral blur with the given kernel
    // -----------------------------------------------------------------------------
    const int   HalfKernelSize      = (KERNEL_SIZE - 1) / 2;
    const float KernelWeights[KERNEL_SIZE] = float[KERNEL_SIZE]( 0.106004, 0.109777, 0.11255, 0.114253, 0.114825, 0.114253, 0.112553, 0.109777, 0.106004);

    const float bZ = 1.0 / normpdf(0.0, BSIGMA);

    for (int IndexOfKernelStep = 0; IndexOfKernelStep <= KERNEL_SIZE; ++IndexOfKernelStep)
    {
        int PixelPosition = IndexOfKernelStep - HalfKernelSize;
        
        CurrentFragment = imageLoad(cs_InputImage, ivec2(X, Y) + ivec2(in_Direction.xy) * ivec2(PixelPosition, PixelPosition)).r;
        
        if (!isnan(CurrentFragment) && !isinf(CurrentFragment))
        {
            FactorR1 = normpdf(CurrentFragment - CenterFragment, BSIGMA);
            FactorR2 = bZ * KernelWeights[IndexOfKernelStep] * KernelWeights[IndexOfKernelStep];

            Factor = FactorR1 * FactorR2;
            
            WeightSum     += Factor;
            FinalFragment += Factor * CurrentFragment;
        }
    }

    FinalFragment = FinalFragment / WeightSum;

    imageStore(cs_OutputImage, ivec2(X, Y), vec4(FinalFragment));
}

#endif // __INCLUDE_CS_BILATERAL_BLUR_GLSL__