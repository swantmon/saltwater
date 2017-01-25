
#ifndef __INCLUDE_FS_BLOOM_GLSL__
#define __INCLUDE_FS_BLOOM_GLSL__

// -----------------------------------------------------------------------------
// Input from engine
// -----------------------------------------------------------------------------
layout(std140, binding = 1) uniform UB0
{
    vec4 ps_ConstantValues0;
    vec4 ps_ConstantValues1;
};

layout (binding = 0) uniform sampler2D ps_InputTexture0;
layout (binding = 2) uniform sampler2D ps_InputTexture1;

// -----------------------------------------------------------------------------
// Input to fragment from VS
// -----------------------------------------------------------------------------
layout(location = 2) in vec2 in_UV;

// -----------------------------------------------------------------------------
// Output to fragment
// -----------------------------------------------------------------------------
layout(location = 0) out vec4 out_Output;

// -------------------------------------------------------------------------------------
// Functions
// -------------------------------------------------------------------------------------
float Luminance(in vec3 _LinearColor)
{
    return dot(_LinearColor, vec3(0.3f, 0.59f, 0.11f));
}

// -----------------------------------------------------------------------------

void main()
{
    vec4  SceneColor;
    vec4  Blur;
    vec3  LinearColor;
    vec3  CombinedBloom;
    vec3  BlurColor;
    float TotalLuminance;
    float BloomLuminance;
    float BloomAmount;
    float ExposureScale;

    // -----------------------------------------------------------------------------
    // Settings
    // -----------------------------------------------------------------------------
    float ps_BloomTreshhold = ps_ConstantValues0.x;
    float ps_ExposureScale  = ps_ConstantValues0.w;
    vec3  ps_BloomTint      = ps_ConstantValues1.xyz;

    // -----------------------------------------------------------------------------
    // Read current data
    // TODO: add bloom dirt mask
    // -----------------------------------------------------------------------------
    SceneColor = texture(ps_InputTexture0, in_UV);
    Blur       = texture(ps_InputTexture1, in_UV);

    // -----------------------------------------------------------------------------
    // Clamp to avoid artifacts from exceeding fp16 through framebuffer blending 
    // of multiple very bright lights
    // -----------------------------------------------------------------------------
    LinearColor.rgb = min(vec3(256 * 256, 256 * 256, 256 * 256), Blur.rgb);

    // -----------------------------------------------------------------------------
    // Compute total luminance and bloom luminance depending on settings
    // -----------------------------------------------------------------------------
    TotalLuminance = Luminance(LinearColor.rgb) * ps_ExposureScale;
    BloomLuminance = TotalLuminance - ps_BloomTreshhold;

    // -----------------------------------------------------------------------------
    // Mask between 0..1
    // -----------------------------------------------------------------------------
    BloomAmount = clamp(BloomLuminance / 2.0f, 0.0f, 1.0f);

    CombinedBloom = BloomAmount * LinearColor.rgb;

    // -----------------------------------------------------------------------------
    // Combine bloom with scene color
    // TODO: Could be simplified with an adaptive blending.
    // -----------------------------------------------------------------------------
    SceneColor.rgb += CombinedBloom.rgb * ps_BloomTint;

    out_Output = SceneColor; 
}

#endif // __INCLUDE_FS_BLOOM_GLSL__