
#ifndef __INCLUDE_FS_MATERIAL_GLSL_
#define __INCLUDE_FS_MATERIAL_GLSL_

#include "common_gbuffer.glsl"

// -----------------------------------------------------------------------------
// Input from engine
// -----------------------------------------------------------------------------
layout(std140, binding = 2) uniform UB2
{
    vec4  ps_TilingOffset;
    vec4  ps_Color;
    float ps_Roughness;
    float ps_Reflectance;
    float ps_MetalMask;
};

// -----------------------------------------------------------------------------
// Textures
// -----------------------------------------------------------------------------
layout(binding = 0) uniform sampler2D in_TextureDiffuse;
layout(binding = 1) uniform sampler2D in_TextureNormal;
layout(binding = 2) uniform sampler2D in_TextureRoughness;
layout(binding = 3) uniform sampler2D in_TextureMetallic;
layout(binding = 4) uniform sampler2D in_TextureAO;

// -----------------------------------------------------------------------------
// Input to fragment from VS
// -----------------------------------------------------------------------------
layout(location = 0) in vec3 in_WSPosition;
layout(location = 1) in vec3 in_WSNormal;
layout(location = 2) in vec2 in_UV;
layout(location = 3) in mat3 in_WSNormalMatrix;

// -----------------------------------------------------------------------------
// Output to fragment
// -----------------------------------------------------------------------------
layout(location = 0) out vec4 out_GBuffer0;
layout(location = 1) out vec4 out_GBuffer1;
layout(location = 2) out vec4 out_GBuffer2;

// -----------------------------------------------------------------------------
// Name:   PSShaderMaterialDisney
// Alias:  Disney Material
// Desc.:  Physically-based shaded material with color, normal, roughness, 
//         ambient occlusion and metalness texture
// -----------------------------------------------------------------------------
void PSShaderMaterialDisney(void)
{
    SGBuffer GBuffer;

    vec2  UV        = in_UV * ps_TilingOffset.xy + ps_TilingOffset.zw;
    vec3  Color     = ps_Color.xyz;
    vec3  WSNormal  = in_WSNormal;
    float Roughness = ps_Roughness;
    float MetalMask = ps_MetalMask;
    float AO        = 1.0f;

#ifdef USE_TEX_DIFFUSE
    Color *= texture(in_TextureDiffuse, UV).rgb;
#endif // USE_TEX_DIFFUSE

#ifdef USE_TEX_NORMAL
    WSNormal = in_WSNormalMatrix * (texture(in_TextureNormal, UV).rgb * 2.0f - 1.0f);
#endif // USE_TEX_NORMAL

#ifdef USE_TEX_ROUGHNESS
    Roughness *= texture(in_TextureRoughness, UV).r;
#endif // USE_TEX_ROUGHNESS

#ifdef USE_TEX_METALLIC
    MetalMask *= texture(in_TextureMetallic, UV).r;
#endif // USE_TEX_METALLIC

#ifdef USE_TEX_AO
    AO *= texture(in_TextureAO, UV).r;
#endif // USE_TEX_AO

    PackGBuffer(Color, WSNormal, Roughness, vec3(ps_Reflectance), MetalMask, AO, GBuffer);

    out_GBuffer0 = GBuffer.m_Color0;
    out_GBuffer1 = GBuffer.m_Color1;
    out_GBuffer2 = GBuffer.m_Color2;
}

#endif // __INCLUDE_FS_MATERIAL_GLSL_