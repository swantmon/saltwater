
#ifndef __INCLUDE_FS_SHADOW_GLSL__
#define __INCLUDE_FS_SHADOW_GLSL__

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

layout(binding = 0) uniform sampler2D in_PSTextureDiffuse;

// -----------------------------------------------------------------------------
// Input to fragment from VS
// -----------------------------------------------------------------------------
layout(location = 0) in vec3 in_WSPosition;
layout(location = 1) in vec3 in_WSNormal;
layout(location = 2) in vec2 in_UV;

// -----------------------------------------------------------------------------
// Output to fragment
// -----------------------------------------------------------------------------
layout(location = 0) out vec4 out_Position;
layout(location = 1) out vec4 out_Normal;
layout(location = 2) out vec4 out_Flux;

// -----------------------------------------------------------------------------
// Function
// -----------------------------------------------------------------------------
void SM(void)
{

}

// -----------------------------------------------------------------------------

void RSM_COLOR(void)
{
    vec3 Color = ps_Color.xyz;

    out_Position = vec4(in_WSPosition, 1.0f);
    out_Normal   = vec4(normalize(in_WSNormal), 0.0f);
    out_Flux     = vec4(Color, 1.0f);
}

// -----------------------------------------------------------------------------

void RSM_TEX(void)
{
    vec2 UV    = in_UV * ps_TilingOffset.xy + ps_TilingOffset.zw;
    vec3 Color = ps_Color.xyz;

    Color *= texture(in_PSTextureDiffuse, UV).rgb;

    out_Position = vec4(in_WSPosition, 1.0f);
    out_Normal   = vec4(normalize(in_WSNormal), 0.0f);
    out_Flux     = vec4(Color, 1.0f);
}

#endif // __INCLUDE_FS_SHADOW_GLSL__