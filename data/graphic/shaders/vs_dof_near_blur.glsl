
#ifndef __INCLUDE_VS_DOF_NEAR_BLUR_GLSL__
#define __INCLUDE_VS_DOF_NEAR_BLUR_GLSL__

#include "common_global.glsl"

layout(location = 0) in vec3 VertexPosition;

struct SPixelCoords
{
    vec4 m_TexCoords;
};

layout(location = 0) out SPixelCoords PSTexCoords;

out gl_PerVertex
{
    vec4 gl_Position;
};

void main()
{
    vec4 Position = vec4(VertexPosition.xyz, 1.0f);
    
    vec2 TexCoord = vec2(Position.x, 1.0f - Position.y);
    
    // -----------------------------------------------------------------------------
    
    vec4 HalfPixel          = vec4(-0.5f, 0.5f, -0.5f, 0.5f);
    PSTexCoords.m_TexCoords = TexCoord.xxyy + HalfPixel * vec4(ps_InvertedScreensizeAndScreensize.xy / 4.0f, ps_InvertedScreensizeAndScreensize.xy / 4.0f);
    
    // -----------------------------------------------------------------------------
    
    gl_Position = ps_WorldToQuad * Position;
}

#endif // __INCLUDE_VS_DOF_NEAR_BLUR_GLSL__