
#ifndef __INCLUDE_FS_DOWN_SAMPLE_GLSL__
#define __INCLUDE_FS_DOWN_SAMPLE_GLSL__

// -----------------------------------------------------------------------------
// Defines
// -----------------------------------------------------------------------------
// Down sample methods:
// 0: Direct (with NaN & INF check)
// 1: High quality 4 samples
// 2: High quality 4 samples with blur
#define METHOD 1

// -----------------------------------------------------------------------------
// Input from engine
// -----------------------------------------------------------------------------
layout(std140, binding = 1) uniform UB0
{
    vec4 ps_ConstantValues0;
};

layout(binding = 0) uniform sampler2D ps_InputTexture;

// -----------------------------------------------------------------------------
// Input
// -----------------------------------------------------------------------------
layout(location = 2) in vec2 in_UV;

// -----------------------------------------------------------------------------
// Output to fragment
// -----------------------------------------------------------------------------
layout(location = 0) out vec4 out_Output;

// -------------------------------------------------------------------------------------
// Functions
// -------------------------------------------------------------------------------------
void main()
{
    // -----------------------------------------------------------------------------
    // Create an empty fragment
    // -----------------------------------------------------------------------------
    vec4 FinalFragment = vec4(0.0f);

#if METHOD == 0

    // -------------------------------------------------------------------------------------
    // Low quality 1 sample
    // -------------------------------------------------------------------------------------
    FinalFragment = texture(ps_InputTexture, in_UV);

    if (isnan(FinalFragment.x))
    {
        FinalFragment = vec4(FinalFragment.x, 0, 0, 0);
    }

    if (isinf(FinalFragment.x))
    {
        FinalFragment = vec4(0, FinalFragment.x, 0, 0);
    }

#elif METHOD == 1

    // -----------------------------------------------------------------------------
    // Settings
    // -----------------------------------------------------------------------------
    vec2 ps_InvertTexturesize = ps_ConstantValues0.xy;

    // -------------------------------------------------------------------------------------
    // high quality 4 samples
    // -------------------------------------------------------------------------------------
    vec2 DownSampleUV[4];

    // -------------------------------------------------------------------------------------
    // blur during downsample (4x4 kernel) to get better quality especially for HDR 
    // content, can be made an option of this shader
    // -------------------------------------------------------------------------------------
    DownSampleUV[0] = in_UV + ps_InvertTexturesize * vec2(-1.0f, -1.0f);
    DownSampleUV[1] = in_UV + ps_InvertTexturesize * vec2( 1.0f, -1.0f);
    DownSampleUV[2] = in_UV + ps_InvertTexturesize * vec2(-1.0f,  1.0f);
    DownSampleUV[3] = in_UV + ps_InvertTexturesize * vec2( 1.0f,  1.0f);

    vec4 Sample[4];

    for(uint IndexOfSample = 0; IndexOfSample < 4; ++ IndexOfSample)
    {
        Sample[IndexOfSample] = texture(ps_InputTexture, DownSampleUV[IndexOfSample]);
    }

    FinalFragment = (Sample[0] + Sample[1] + Sample[2] + Sample[3]) * 0.25f;

    // -------------------------------------------------------------------------------------
    // fixed rarely occuring yellow color tine of the whole viewport (certain view port 
    // size, need to investigate more)
    // -------------------------------------------------------------------------------------
    FinalFragment.rgb = max(vec3(0.0f, 0.0f, 0.0f), FinalFragment.rgb);

#else

    // -----------------------------------------------------------------------------
    // Settings
    // -----------------------------------------------------------------------------
    vec2 ps_InvertTexturesize = ps_ConstantValues0.xy;

    // -------------------------------------------------------------------------------------
    // high quality 4 samples with blurring
    // Depth in Alpha
    // -------------------------------------------------------------------------------------
    vec2 DownSampleUV[4];

    // -------------------------------------------------------------------------------------
    // no filtering (2x2 kernel) to get no leaking in Depth of Field
    // -------------------------------------------------------------------------------------
    DownSampleUV[0] = in_UV + ps_InvertTexturesize * vec2(-0.5f, -0.5f);
    DownSampleUV[1] = in_UV + ps_InvertTexturesize * vec2( 0.5f, -0.5f);
    DownSampleUV[2] = in_UV + ps_InvertTexturesize * vec2(-0.5f,  0.5f);
    DownSampleUV[3] = in_UV + ps_InvertTexturesize * vec2( 0.5f,  0.5f);

    vec3 Sample[4];

    for(uint IndexOfSample = 0; IndexOfSample < 4; ++ IndexOfSample)
    {
        Sample[IndexOfSample] = texture(ps_InputTexture, DownSampleUV[IndexOfSample]).rgb;
    }

    // -------------------------------------------------------------------------------------
    // todo: this still leaks as we need to mask in focus pixels
    // -------------------------------------------------------------------------------------
    FinalFragment = vec4((Sample[0] + Sample[1] + Sample[2] + Sample[3]) * 0.25f, 0.0f);

#endif

    // -----------------------------------------------------------------------------
    // Return final color
    // -----------------------------------------------------------------------------
    out_Output = FinalFragment;
}

#endif // __INCLUDE_FS_DOWN_SAMPLE_GLSL__