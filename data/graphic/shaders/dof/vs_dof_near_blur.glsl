
#ifndef __INCLUDE_VS_DOF_NEAR_BLUR_GLSL__
#define __INCLUDE_VS_DOF_NEAR_BLUR_GLSL__

#include "common_global.glsl"

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
    vec2 Vertices[3];
    
    Vertices[0] = vec2(-1.0f, -1.0f);
    Vertices[1] = vec2( 3.0f, -1.0f);
    Vertices[2] = vec2(-1.0f,  3.0f);

    vec2 UV = Vertices[gl_VertexID] * 0.5f + 0.5f;
    
    // -----------------------------------------------------------------------------
    
    vec4 HalfPixel          = vec4(-0.5f, 0.5f, -0.5f, 0.5f);
    PSTexCoords.m_TexCoords = UV.xxyy + HalfPixel * vec4(g_InvertedScreensizeAndScreensize.xy / 4.0f, g_InvertedScreensizeAndScreensize.xy / 4.0f);
    
    // -----------------------------------------------------------------------------
    
    gl_Position = vec4(Vertices[gl_VertexID], 0.0f, 1.0f);
}

#endif // __INCLUDE_VS_DOF_NEAR_BLUR_GLSL__