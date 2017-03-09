
#ifndef __INCLUDE_FS_SPHERICAL_ENV_CUBEMAP_GENERATION_GLSL__
#define __INCLUDE_FS_SPHERICAL_ENV_CUBEMAP_GENERATION_GLSL__

#include "common.glsl"

// -----------------------------------------------------------------------------
// Input from engine
// -----------------------------------------------------------------------------
layout(row_major, std140, binding = 4) uniform UB0
{
    vec4 m_ConstantBufferData0;
};

layout(binding = 0) uniform sampler2D PSEnvironmentTexture;

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
// Functions
// -----------------------------------------------------------------------------
vec4 GetTextureSpherical(in vec3 _Normal, in float _LOD)
{
    float NormalLength = length(_Normal.xz);

    vec2 TexCoord = vec2((NormalLength > 0.0000001f) ? _Normal.x / NormalLength : 0.0f, _Normal.y);

    TexCoord = acos(TexCoord) * INV_PI;
    
    TexCoord.x = (_Normal.z > 0.0) ? TexCoord.x * 0.5f : 1.0f - (TexCoord.x * 0.5f);

    return textureLod(PSEnvironmentTexture, TexCoord, _LOD);
}

// -----------------------------------------------------------------------------
// Main
// -----------------------------------------------------------------------------
void main(void)
{
    vec4 FinalColor = GetTextureSpherical(in_Normal, 0);
    
    if (m_IsHDR == 0.0f)
    {
        FinalColor = InverseToneMapping(FinalColor);
    }
    
    out_Output = vec4(clamp(FinalColor.xyz * m_HDRConvertFactor, 0.0f, F16_MAX), 1.0f);
}

#endif // __INCLUDE_FS_SPHERICAL_ENV_CUBEMAP_GENERATION_GLSL__