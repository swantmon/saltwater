
#ifndef __INCLUDE_VS_DOF_DOWN_SAMPLE_GLSL__
#define __INCLUDE_VS_DOF_DOWN_SAMPLE_GLSL__

#include "common_global.glsl"

struct SPixelCoords
{
    vec2 m_Color0;
    vec2 m_Color1;
    vec2 m_Depth0;
    vec2 m_Depth1;
    vec2 m_Depth2;
    vec2 m_Depth3;
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
    
    PSTexCoords.m_Color0 = UV + vec2(-1.0f, -1.0f) * g_InvertedScreensizeAndScreensize.xy;
    PSTexCoords.m_Color1 = UV + vec2(+1.0f, -1.0f) * g_InvertedScreensizeAndScreensize.xy;
    
    PSTexCoords.m_Depth0 = UV + vec2(-1.5f, -1.5f) * g_InvertedScreensizeAndScreensize.xy;
    PSTexCoords.m_Depth1 = UV + vec2(-0.5f, -1.5f) * g_InvertedScreensizeAndScreensize.xy;
    PSTexCoords.m_Depth2 = UV + vec2(+0.5f, -1.5f) * g_InvertedScreensizeAndScreensize.xy;
    PSTexCoords.m_Depth3 = UV + vec2(+1.5f, -1.5f) * g_InvertedScreensizeAndScreensize.xy;
    
    // -----------------------------------------------------------------------------

    gl_Position = vec4(Vertices[gl_VertexID], 0.0f, 1.0f);
}

#endif // __INCLUDE_VS_DOF_DOWN_SAMPLE_GLSL__