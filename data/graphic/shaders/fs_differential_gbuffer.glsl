
#ifndef __INCLUDE_FS_DIFFERENTIAL_SCENE_GLSL_
#define __INCLUDE_FS_DIFFERENTIAL_SCENE_GLSL_

#include "common_gbuffer.glsl"
#include "common_global.glsl"

// -----------------------------------------------------------------------------
// Input from engine
// -----------------------------------------------------------------------------
layout(std140, binding = 2) uniform UB2
{
    vec4  ps_TilingOffset;
    vec3  ps_Color;
    float ps_Roughness;
    float ps_Reflectance;
    float ps_MetalMask;
};

// -----------------------------------------------------------------------------
// Input from engine
// -----------------------------------------------------------------------------
layout (binding = 0) uniform sampler2D ps_BackgroundColor;

// -----------------------------------------------------------------------------
// Input from previous stage
// -----------------------------------------------------------------------------
layout(location = 0) in vec3 in_WSPosition;
layout(location = 1) in vec3 in_WSNormal;
layout(location = 2) in vec2 in_UV;
layout(location = 3) in mat3 in_WSNormalMatrix;

// -----------------------------------------------------------------------------
// Output
// -----------------------------------------------------------------------------
layout(location = 0) out vec4 out_GBuffer0;
layout(location = 1) out vec4 out_GBuffer1;
layout(location = 2) out vec4 out_GBuffer2;

// -----------------------------------------------------------------------------
// Function
// -----------------------------------------------------------------------------
void main(void)
{
    // -----------------------------------------------------------------------------
    // Define tex coords from system input
    // -----------------------------------------------------------------------------
    vec2 TexCoord = vec2(gl_FragCoord.x * g_InvertedScreensizeAndScreensize.x, 1.0f - gl_FragCoord.y * g_InvertedScreensizeAndScreensize.y);
    
    vec3 Color = texture(ps_BackgroundColor, TexCoord).rgb * ps_Color;

    SGBuffer GBuffer;
    
    PackGBuffer(Color, in_WSNormal, ps_Roughness, vec3(ps_Reflectance), ps_MetalMask, 1.0f, GBuffer);

    out_GBuffer0 = GBuffer.m_Color0;
    out_GBuffer1 = GBuffer.m_Color1;
    out_GBuffer2 = GBuffer.m_Color2;
}

#endif // __INCLUDE_FS_DIFFERENTIAL_SCENE_GLSL_