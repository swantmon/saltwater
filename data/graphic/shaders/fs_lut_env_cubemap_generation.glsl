
#ifndef __INCLUDE_FS_LUT_ENV_CUBEMAP_GENERATION_GLSL__
#define __INCLUDE_FS_LUT_ENV_CUBEMAP_GENERATION_GLSL__

#include "common.glsl"

// -----------------------------------------------------------------------------
// Input from engine
// -----------------------------------------------------------------------------
layout(row_major, std140, binding = 0) uniform UB0
{
    vec4 m_ConstantBufferData0;
};

layout(binding = 0) uniform sampler2D PSBackgroundTexture;
layout(binding = 1) uniform samplerCube PSLookUpTexture;

// -----------------------------------------------------------------------------
// Easy access defines
// -----------------------------------------------------------------------------
#define m_HDRConvertFactor  m_ConstantBufferData0.x
#define m_IsHDR             m_ConstantBufferData0.y

// -----------------------------------------------------------------------------
// Input to fragment from VS
// -----------------------------------------------------------------------------
layout(location = 0) in vec3 in_Normal;
layout(location = 1) in vec2 in_UV;

// -----------------------------------------------------------------------------
// Output to fragment
// -----------------------------------------------------------------------------
layout(location = 0) out vec4 out_Output;

// -----------------------------------------------------------------------------
// Main
// -----------------------------------------------------------------------------
void main(void)
{
	vec4 LookUp = textureLod(PSLookUpTexture, in_Normal, 0);

    vec4 FinalColor = texture(PSBackgroundTexture, LookUp.xy);
    
    if (m_IsHDR == 0.0f)
    {
        FinalColor = InverseToneMapping(FinalColor);
    }
    
    out_Output = FinalColor * m_HDRConvertFactor;
}

#endif // __INCLUDE_FS_LUT_ENV_CUBEMAP_GENERATION_GLSL__