
#ifndef __INCLUDE_FS_ATMOSPHERE_CUBEMAP_GLSL__
#define __INCLUDE_FS_ATMOSPHERE_CUBEMAP_GLSL__

#include "common.glsl"

// -----------------------------------------------------------------------------
// Input from engine
// -----------------------------------------------------------------------------
layout(row_major, std140, binding = 8) uniform UB8
{
    vec4 m_ConstantBufferData0;
};

layout(std430, binding = 0) readonly buffer BB0
{
    float ps_ExposureHistory[8];
};

layout(binding = 0) uniform sampler2D ps_EnvironmentTexture;
layout(binding = 1) uniform sampler2D ps_Depth;

// -----------------------------------------------------------------------------
// Easy access defines
// -----------------------------------------------------------------------------
#define m_HDRConvertFactor  m_ConstantBufferData0.x
#define m_IsHDR             m_ConstantBufferData0.y
#define m_ExposureIndex     m_ConstantBufferData0.z

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
    out_Output = texture(ps_EnvironmentTexture, in_UV); return;


    // -----------------------------------------------------------------------------
    // Check Depth
    // -----------------------------------------------------------------------------
    float VSDepth = texture(ps_Depth, in_UV).r;
    
    if (VSDepth < 1.0f)
    {
        discard;
    }

    // -----------------------------------------------------------------------------
    // Exposure data
    // -----------------------------------------------------------------------------    
    float AverageExposure = ps_ExposureHistory[uint(m_ExposureIndex)];
    
    // -----------------------------------------------------------------------------
    // Lighting only if depth is infinity
    // -----------------------------------------------------------------------------
    vec4 FinalColor = texture(ps_EnvironmentTexture, in_UV);

    if (m_IsHDR == 0.0f)
    {
        FinalColor = InverseToneMapping(FinalColor);
    }
    
    out_Output = FinalColor * m_HDRConvertFactor * AverageExposure;
}

#endif // __INCLUDE_FS_ATMOSPHERE_CUBEMAP_GLSL__