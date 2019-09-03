
#ifndef __INCLUDE_FS_SPHERICAL_CUBEMAP_GENERATION_GLSL__
#define __INCLUDE_FS_SPHERICAL_CUBEMAP_GENERATION_GLSL__

#include "common.glsl"

// -----------------------------------------------------------------------------
// Input from engine
// -----------------------------------------------------------------------------
layout(std140, binding = 4) uniform UB0
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
#define VERSION 0

#if VERSION = 0
vec4 GetTextureSpherical(in vec3 _Normal, in float _LOD)
{
    float NormalLength = length(_Normal.xz);

    vec2 TexCoord = vec2((NormalLength > 0.0000001f) ? _Normal.x / NormalLength : 0.0f, _Normal.y);

    TexCoord = acos(TexCoord) * INV_PI;
    
    TexCoord.x = (_Normal.z > 0.0) ? 1.0f - (TexCoord.x * 0.5f) : (TexCoord.x * 0.5f);

    TexCoord.y = 1.0f - TexCoord.y;

    return textureLod(PSEnvironmentTexture, TexCoord, _LOD);
}
#elif VERSION = 1
vec4 GetTextureSpherical(in vec3 _Normal, in float _LOD)
{
    float Phi   = acos(_Normal.y);
    float Theta = atan(-1.0f * _Normal.x, _Normal.z) + PI;

    vec2 UV = vec2(Theta / (2.0f * PI), 1.0f - Phi / PI);

    return textureLod(PSEnvironmentTexture, UV, _LOD);
}
#endif

// -----------------------------------------------------------------------------
// Main
// -----------------------------------------------------------------------------
void main(void)
{
    vec4 FinalColor = GetTextureSpherical(-in_Normal, 0.0f);
    
    if (m_IsHDR == 0.0f)
    {
        FinalColor = InverseToneMapping(FinalColor);
    }
    
    out_Output = vec4(clamp(FinalColor.xyz * m_HDRConvertFactor, 0.0f, F16_MAX), 1.0f);
}

#endif // __INCLUDE_FS_SPHERICAL_CUBEMAP_GENERATION_GLSL__