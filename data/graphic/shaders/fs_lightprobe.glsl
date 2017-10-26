
#ifndef __INCLUDE_FS_LIGHTPROBE_GLSL__
#define __INCLUDE_FS_LIGHTPROBE_GLSL__

#include "common.glsl"

// -----------------------------------------------------------------------------
// Input from engine
// -----------------------------------------------------------------------------
layout(std140, binding = 3) uniform UB3
{
    vec4 ps_ConstantBufferData0;
};

layout(std430, binding = 0) buffer BB0
{
    float ps_ExposureHistory[8];
};

layout(binding = 0) uniform samplerCube PSEnvironmentTexture;

// -----------------------------------------------------------------------------
// Easy access
// -----------------------------------------------------------------------------
#define ps_ExposureHistoryIndex ps_ConstantBufferData0.x
#define ps_UseExposure          ps_ConstantBufferData0.y

// -----------------------------------------------------------------------------
// Input to fragment from previous stage
// -----------------------------------------------------------------------------
layout(location = 1) in vec3 in_Normal;

// -----------------------------------------------------------------------------
// Output to fragment
// -----------------------------------------------------------------------------
layout(location = 0) out vec4 out_Output;

// -----------------------------------------------------------------------------
// Main
// -----------------------------------------------------------------------------
void main(void)
{
    // -----------------------------------------------------------------------------
    // Get color from cube map
    // -----------------------------------------------------------------------------
    vec4 FinalColor = textureLod(PSEnvironmentTexture, in_Normal, 0.0f);

    // -----------------------------------------------------------------------------
    // Exposure data
    // -----------------------------------------------------------------------------
    float AverageExposure = ps_ExposureHistory[uint(ps_ExposureHistoryIndex)];

    // -------------------------------------------------------------------------------------
    // Output
    // ------------------------------------------------------------------------------------- 
    out_Output = vec4(clamp(FinalColor.xyz * AverageExposure, 0.0f, F16_MAX), 1.0f);
}

#endif // __INCLUDE_FS_LIGHTPROBE_GLSL__