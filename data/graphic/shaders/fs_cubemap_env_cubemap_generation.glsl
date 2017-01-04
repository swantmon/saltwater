
#ifndef __INCLUDE_FS_CUBEMAP_ENV_CUBEMAP_GENERATION_GLSL__
#define __INCLUDE_FS_CUBEMAP_ENV_CUBEMAP_GENERATION_GLSL__

#include "common.glsl"

// -----------------------------------------------------------------------------
// Input from engine
// -----------------------------------------------------------------------------
layout(row_major, std140, binding = 0) uniform UB0
{
    vec4 m_ConstantBufferData0;
};

layout(binding = 0) uniform samplerCube PSEnvironmentTexture;

// -----------------------------------------------------------------------------
// Easy access defines
// -----------------------------------------------------------------------------
#define m_HDRConvertFactor  m_ConstantBufferData0.x
#define m_IsHDR             m_ConstantBufferData0.y

// -----------------------------------------------------------------------------
// Input to fragment from VS
// -----------------------------------------------------------------------------
layout(location = 0) in vec3 in_Normal;

// -----------------------------------------------------------------------------
// Output to fragment
// -----------------------------------------------------------------------------
layout(location = 0) out vec4 out_Output;

// -----------------------------------------------------------------------------
// Main
// -----------------------------------------------------------------------------
void main(void)
{
    vec4 FinalColor = textureLod(PSEnvironmentTexture, in_Normal, 0);

    if (m_IsHDR == 0.0f)
    {
        FinalColor = InverseToneMapping(FinalColor);
    }
    
    out_Output = vec4(FinalColor.xyz * m_HDRConvertFactor, 1.0f);
}

#endif // __INCLUDE_FS_CUBEMAP_ENV_CUBEMAP_GENERATION_GLSL__