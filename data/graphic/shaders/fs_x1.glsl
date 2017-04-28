
#ifndef __INCLUDE_FS_X1_GLSL__
#define __INCLUDE_FS_X1_GLSL__

// -----------------------------------------------------------------------------
// Input from engine
// -----------------------------------------------------------------------------
layout(std140, binding = 3) uniform UB0
{
    vec4  ps_TilingOffset;
    vec3  ps_Color;
    float ps_Roughness;
    float ps_Reflectance;
    float ps_MetalMask;
};

layout(binding = 0) uniform sampler2D PSTextureDiffuse;

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
    vec2 UV    = in_UV * ps_TilingOffset.xy + ps_TilingOffset.zw;
    vec3 Color = ps_Color.xyz;

#ifdef USE_TEX_DIFFUSE
    Color *= texture(PSTextureDiffuse, UV).rgb;
#endif // USE_TEX_DIFFUSE

    out_Output = vec4(Color * 10000.0f, 0.0f);
}

#endif // __INCLUDE_FS_X1_GLSL__