
#ifndef __INCLUDE_FS_TEXTURE_ENV_CUBEMAP_GENERATION_GLSL__
#define __INCLUDE_FS_TEXTURE_ENV_CUBEMAP_GENERATION_GLSL__

#include "common.glsl"
#include "common_global.glsl"

// -----------------------------------------------------------------------------
// Input from engine
// -----------------------------------------------------------------------------
layout(row_major, std140, binding = 1) uniform UB1
{
    vec4 m_ConstantBufferData0;
};

layout(binding = 0) uniform sampler2D PSBackgroundTexture;

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
	vec2  SSUV   = -1.0f + 2.0f * in_UV;
  	float Radius = min(sqrt(dot(SSUV, SSUV)), 1.0f);

    vec4 FinalColor = texture(PSBackgroundTexture, in_UV);
    
    if (m_IsHDR == 0.0f)
    {
        FinalColor = InverseToneMapping(FinalColor);
    }
    
    out_Output = vec4(clamp(FinalColor.xyz * m_HDRConvertFactor, 0.0f, F16_MAX), min((1.0f - Radius) + g_FrameDeltaTime, 1.0f));
}

#endif // __INCLUDE_FS_TEXTURE_ENV_CUBEMAP_GENERATION_GLSL__