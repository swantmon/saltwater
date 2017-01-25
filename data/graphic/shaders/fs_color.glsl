
#ifndef __INCLUDE_FS_COLOR_GLSL_
#define __INCLUDE_FS_COLOR_GLSL_

// -----------------------------------------------------------------------------
// Input from engine
// -----------------------------------------------------------------------------
layout(std140, binding = 1) uniform UB0
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
layout(binding = 3) uniform sampler2D PSTextureReflectance;
layout(binding = 4) uniform sampler2D PSTextureMetallic;
layout(binding = 5) uniform sampler2D PSTextureAO;

// -----------------------------------------------------------------------------
// Input to fragment from VS
// -----------------------------------------------------------------------------
layout(location = 1) in vec3 in_PSNormal;
layout(location = 2) in vec2 in_PSTexCoord;
layout(location = 3) in mat3 in_PSWSNormalMatrix;

// -----------------------------------------------------------------------------
// Output to fragment
// -----------------------------------------------------------------------------
layout(location = 0) out vec4 out_Color;

// -----------------------------------------------------------------------------
// Name:   PSShaderMaterialDisneyBlank
// Alias:  Disney Material
// Desc.:  Physically-based shaded material with no textures
// -----------------------------------------------------------------------------
void main(void)
{
    vec3 WSNormal       = in_PSNormal;
    vec3 LightDirection = vec3(0.0f, -0.2f, -1.0f);
    
    LightDirection = normalize(-LightDirection);
    WSNormal       = normalize(WSNormal);

    vec3 Ambient = ps_Color * 0.4f;
    vec3 Diffuse = ps_Color * max(dot(WSNormal, LightDirection), 0.0f);

    out_Color = vec4(Ambient + Diffuse, 1.0f);
}

#endif // __INCLUDE_FS_COLOR_GLSL_