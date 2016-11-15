
#ifndef __INCLUDE_FS_MATERIAL_GLSL_
#define __INCLUDE_FS_MATERIAL_GLSL_

#include "common_gbuffer.glsl"

// -----------------------------------------------------------------------------
// Input from engine
// -----------------------------------------------------------------------------
layout(std140, binding = 0) uniform UB0
{
    vec4  ps_TilingOffset;
    vec3  ps_Color;
    float ps_Roughness;
    float ps_Reflectance;
    float ps_MetalMask;
};

// -----------------------------------------------------------------------------
// Textures
// -----------------------------------------------------------------------------
layout(binding = 0) uniform sampler2D PSTextureDiffuse;
layout(binding = 1) uniform sampler2D PSTextureNormal;
layout(binding = 2) uniform sampler2D PSTextureRoughness;
layout(binding = 3) uniform sampler2D PSTextureMetallic;
layout(binding = 4) uniform sampler2D PSTextureAO;

// -----------------------------------------------------------------------------
// Input to fragment from VS
// -----------------------------------------------------------------------------
layout(location = 1) in vec3 in_PSNormal;
layout(location = 2) in vec2 in_PSTexCoord;
layout(location = 3) in mat3 in_PSWSNormalMatrix;

// -----------------------------------------------------------------------------
// Output to fragment
// -----------------------------------------------------------------------------
layout(location = 0) out vec4 out_GBuffer0;
layout(location = 1) out vec4 out_GBuffer1;
layout(location = 2) out vec4 out_GBuffer2;

// -----------------------------------------------------------------------------
// Name:   PSShaderMaterialDisneyBlank
// Alias:  Disney Material
// Desc.:  Physically-based shaded material with no textures
// -----------------------------------------------------------------------------
void PSShaderMaterialDisneyBlank(void)
{
    SGBuffer GBuffer;
    
    PackGBuffer(ps_Color.xyz, in_PSNormal, ps_Roughness, vec3(ps_Reflectance), ps_MetalMask, 1.0f, GBuffer);

    out_GBuffer0 = GBuffer.m_Color0;
    out_GBuffer1 = GBuffer.m_Color1;
    out_GBuffer2 = GBuffer.m_Color2;
}

// -----------------------------------------------------------------------------
// Name:   PSShaderMaterialDisneyNX0
// Alias:  Disney Material
// Desc.:  Physically-based shaded material with normal
// -----------------------------------------------------------------------------
void PSShaderMaterialDisneyNX0(void)
{
    SGBuffer GBuffer;

    vec3 WSNormal = in_PSWSNormalMatrix * (texture(PSTextureNormal, in_PSTexCoord * ps_TilingOffset.xy + ps_TilingOffset.zw).rgb * 2.0f - 1.0f);
    
    PackGBuffer(ps_Color.xyz, WSNormal, ps_Roughness, vec3(ps_Reflectance), ps_MetalMask, 1.0f, GBuffer);

    out_GBuffer0 = GBuffer.m_Color0;
    out_GBuffer1 = GBuffer.m_Color1;
    out_GBuffer2 = GBuffer.m_Color2;
}

// -----------------------------------------------------------------------------
// Name:   PSShaderMaterialDisneyCX0
// Alias:  Disney Material
// Desc.:  Physically-based shaded material with color
// -----------------------------------------------------------------------------
void PSShaderMaterialDisneyCX0(void)
{
    SGBuffer GBuffer;

    vec3 Color = texture(PSTextureDiffuse, in_PSTexCoord * ps_TilingOffset.xy + ps_TilingOffset.zw).rgb * ps_Color.xyz;
    
    PackGBuffer(Color, in_PSNormal, ps_Roughness, vec3(ps_Reflectance), ps_MetalMask, 1.0f, GBuffer);

    out_GBuffer0 = GBuffer.m_Color0;
    out_GBuffer1 = GBuffer.m_Color1;
    out_GBuffer2 = GBuffer.m_Color2;
}

// -----------------------------------------------------------------------------
// Name:   PSShaderMaterialDisneyCNX0
// Alias:  Disney Material
// Desc.:  Physically-based shaded material with color, normal
// -----------------------------------------------------------------------------
void PSShaderMaterialDisneyCNX0(void)
{
    SGBuffer GBuffer;

    vec3  WSNormal  = in_PSWSNormalMatrix * (texture(PSTextureNormal, in_PSTexCoord * ps_TilingOffset.xy + ps_TilingOffset.zw).rgb * 2.0f - 1.0f);
    vec3  Color     = texture(PSTextureDiffuse, in_PSTexCoord * ps_TilingOffset.xy + ps_TilingOffset.zw).rgb * ps_Color.xyz;

    PackGBuffer(Color, WSNormal, ps_Roughness, vec3(ps_Reflectance), ps_MetalMask, 1.0f, GBuffer);

    out_GBuffer0 = GBuffer.m_Color0;
    out_GBuffer1 = GBuffer.m_Color1;
    out_GBuffer2 = GBuffer.m_Color2;
}

// -----------------------------------------------------------------------------
// Name:   PSShaderMaterialDisneyCNRX0
// Alias:  Disney Material
// Desc.:  Physically-based shaded material with color, normal and roughness
// -----------------------------------------------------------------------------
void PSShaderMaterialDisneyCNRX0(void)
{
    SGBuffer GBuffer;

    vec3  WSNormal  = in_PSWSNormalMatrix * (texture(PSTextureNormal, in_PSTexCoord * ps_TilingOffset.xy + ps_TilingOffset.zw).rgb * 2.0f - 1.0f);
    vec3  Color     = texture(PSTextureDiffuse  , in_PSTexCoord * ps_TilingOffset.xy + ps_TilingOffset.zw).rgb * ps_Color.xyz;
    float Roughness = texture(PSTextureRoughness, in_PSTexCoord * ps_TilingOffset.xy + ps_TilingOffset.zw).r   * ps_Roughness;
    
    PackGBuffer(Color, WSNormal, Roughness, vec3(ps_Reflectance), ps_MetalMask, 1.0f, GBuffer);

    out_GBuffer0 = GBuffer.m_Color0;
    out_GBuffer1 = GBuffer.m_Color1;
    out_GBuffer2 = GBuffer.m_Color2;
}

// -----------------------------------------------------------------------------
// Name:   PSShaderMaterialDisneyCNRMX0
// Alias:  Disney Material
// Desc.:  Physically-based shaded material with color, normal, roughness 
//         and metalness texture
// -----------------------------------------------------------------------------
void PSShaderMaterialDisneyCNRMX0(void)
{
    SGBuffer GBuffer;

    vec3  WSNormal  = in_PSWSNormalMatrix * (texture(PSTextureNormal, in_PSTexCoord * ps_TilingOffset.xy + ps_TilingOffset.zw).rgb * 2.0f - 1.0f);
    vec3  Color     = texture(PSTextureDiffuse  , in_PSTexCoord * ps_TilingOffset.xy + ps_TilingOffset.zw).rgb * ps_Color.xyz;
    float Roughness = texture(PSTextureRoughness, in_PSTexCoord * ps_TilingOffset.xy + ps_TilingOffset.zw).r   * ps_Roughness;
    float MetalMask = texture(PSTextureMetallic , in_PSTexCoord * ps_TilingOffset.xy + ps_TilingOffset.zw).r   * ps_MetalMask;
    
    PackGBuffer(Color, WSNormal, Roughness, vec3(ps_Reflectance), MetalMask, 1.0f, GBuffer);

    out_GBuffer0 = GBuffer.m_Color0;
    out_GBuffer1 = GBuffer.m_Color1;
    out_GBuffer2 = GBuffer.m_Color2;
}

// -----------------------------------------------------------------------------
// Name:   PSShaderMaterialDisneyCNRMAX0
// Alias:  Disney Material
// Desc.:  Physically-based shaded material with color, normal, roughness, 
//         ambient occlusion and metalness texture
// -----------------------------------------------------------------------------
void PSShaderMaterialDisneyCNRMAX0(void)
{
    SGBuffer GBuffer;

    vec3  WSNormal  = in_PSWSNormalMatrix * (texture(PSTextureNormal, in_PSTexCoord * ps_TilingOffset.xy + ps_TilingOffset.zw).rgb * 2.0f - 1.0f);
    vec3  Color     = texture(PSTextureDiffuse  , in_PSTexCoord * ps_TilingOffset.xy + ps_TilingOffset.zw).rgb * ps_Color.xyz;
    float Roughness = texture(PSTextureRoughness, in_PSTexCoord * ps_TilingOffset.xy + ps_TilingOffset.zw).r   * ps_Roughness;
    float MetalMask = texture(PSTextureMetallic , in_PSTexCoord * ps_TilingOffset.xy + ps_TilingOffset.zw).r   * ps_MetalMask;
    float AO        = texture(PSTextureAO       , in_PSTexCoord * ps_TilingOffset.xy + ps_TilingOffset.zw).r;
    
    PackGBuffer(Color, WSNormal, Roughness, vec3(ps_Reflectance), MetalMask, AO, GBuffer);

    out_GBuffer0 = GBuffer.m_Color0;
    out_GBuffer1 = GBuffer.m_Color1;
    out_GBuffer2 = GBuffer.m_Color2;
}

#endif // __INCLUDE_FS_MATERIAL_GLSL_