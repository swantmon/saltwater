
#ifndef __INCLUDE_VS_DOF_DOWN_SAMPLE_GLSL__
#define __INCLUDE_VS_DOF_DOWN_SAMPLE_GLSL__

#include "common_global.glsl"

layout(location = 0) in vec3 VertexPosition;

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
	vec4 Position = vec4(VertexPosition.xyz, 1.0f);
    
    vec2 TexCoord = vec2(Position.x, 1.0f - Position.y);
    
    // -----------------------------------------------------------------------------
    
    PSTexCoords.m_Color0 = TexCoord + vec2(-1.0f, -1.0f) * ps_InvertedScreensizeAndScreensize.xy;
    PSTexCoords.m_Color1 = TexCoord + vec2(+1.0f, -1.0f) * ps_InvertedScreensizeAndScreensize.xy;
    
    PSTexCoords.m_Depth0 = TexCoord + vec2(-1.5f, -1.5f) * ps_InvertedScreensizeAndScreensize.xy;
    PSTexCoords.m_Depth1 = TexCoord + vec2(-0.5f, -1.5f) * ps_InvertedScreensizeAndScreensize.xy;
    PSTexCoords.m_Depth2 = TexCoord + vec2(+0.5f, -1.5f) * ps_InvertedScreensizeAndScreensize.xy;
    PSTexCoords.m_Depth3 = TexCoord + vec2(+1.5f, -1.5f) * ps_InvertedScreensizeAndScreensize.xy;
    
    // -----------------------------------------------------------------------------

	gl_Position = ps_WorldToQuad * Position;
}

#endif // __INCLUDE_VS_DOF_DOWN_SAMPLE_GLSL__