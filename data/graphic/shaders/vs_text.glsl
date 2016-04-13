
#ifndef __INCLUDE_VS_TEXT_GLSL__
#define __INCLUDE_VS_TEXT_GLSL__

#include "vs_global.glsl"

layout(location = 0) in vec2 VertexPosition;
layout(location = 1) in vec3 TextSettings;
layout(location = 2) in vec3 CharSettings;
layout(location = 3) in vec4 CharColor;

layout(location = 1) out vec4 PSColor;
layout(location = 2) out vec2 PSTexCoord;

out gl_PerVertex
{
    vec4 gl_Position;
};

void main(void)
{
    vec2 InvertedTextSize     = vs_InvertedScreensize.xy * TextSettings.x;
    vec2 InvertedTextPosition = vs_InvertedScreensize.xy * TextSettings.yz;

    vec4 WSPosition = vec4(VertexPosition.xy * InvertedTextSize + InvertedTextPosition, 0.0f, 1.0f);

    PSTexCoord = VertexPosition.xy * CharSettings.x + (CharSettings.yz * CharSettings.x);
    PSColor    = CharColor;
    
    gl_Position = vs_ViewProjectionScreenMatrix * WSPosition;
}

#endif // __INCLUDE_VS_TEXT_GLSL__