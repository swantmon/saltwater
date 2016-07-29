
#ifndef __INCLUDE_FS_DIFFERENTIAL_SCENE_GLSL_
#define __INCLUDE_FS_DIFFERENTIAL_SCENE_GLSL_

#include "common_gbuffer.glsl"
#include "common_global.glsl"

// -----------------------------------------------------------------------------
// Input from engine
// -----------------------------------------------------------------------------
layout(std140, binding = 1) uniform UB1
{
    vec4  ps_TilingOffset;
    vec3  ps_Color;
    float ps_Roughness;
    float ps_Reflectance;
    float ps_MetalMask;
};


// -----------------------------------------------------------------------------
// Input from system
// -----------------------------------------------------------------------------
in vec4 gl_FragCoord;

// -----------------------------------------------------------------------------
// Input from engine
// -----------------------------------------------------------------------------
layout (binding = 0) uniform sampler2D ps_BackgroundColor;

// -----------------------------------------------------------------------------
// Input to fragment from VS
// -----------------------------------------------------------------------------
layout(location = 1) in vec3 in_PSNormal;

// -----------------------------------------------------------------------------
// Output to fragment
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
    vec2 TexCoord = vec2(gl_FragCoord.x * ps_InvertedScreensizeAndScreensize.x, 1.0f - gl_FragCoord.y * ps_InvertedScreensizeAndScreensize.y);
    
    vec3 Color = texture(ps_BackgroundColor, TexCoord).rgb * ps_Color;

    SGBuffer GBuffer;
    
    PackGBuffer(Color, in_PSNormal, ps_Roughness, vec3(ps_Reflectance), ps_MetalMask, 1.0f, GBuffer);

    out_GBuffer0 = GBuffer.m_Color0;
    out_GBuffer1 = GBuffer.m_Color1;
    out_GBuffer2 = GBuffer.m_Color2;
}

#endif // __INCLUDE_FS_DIFFERENTIAL_SCENE_GLSL_