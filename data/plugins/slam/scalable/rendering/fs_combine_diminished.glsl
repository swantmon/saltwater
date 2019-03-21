
#ifndef __INCLUDE_FS_OUTLINE_GLSL__
#define __INCLUDE_FS_OUTLINE_GLSL__

layout(binding = 0) uniform sampler2D g_BackgroundImage;
layout(binding = 1) uniform sampler2D g_DiminishedImage;
layout(binding = 2) uniform sampler2D g_Membrane;


layout(std140, binding = 0) uniform RGBConversion
{
    mat4 Unused;
    float g_Flipped;
    vec3 Unused2;
};

layout(location = 0) in vec2 in_TexCoords;
layout(location = 1) in vec2 in_TexCoordsFlipped;

layout(location = 0) out vec4 out_Color;

void main()
{
    vec4 Background = texture(g_BackgroundImage, in_TexCoords);
    vec4 Diminished = texture(g_DiminishedImage, in_TexCoordsFlipped);
    vec4 Membrane = texture(g_Membrane, in_TexCoordsFlipped);
    
    float s1 = textureLod(g_DiminishedImage, in_TexCoordsFlipped + vec2(-0.01f, 0.0f), 1.0f).a;
    float s2 = textureLod(g_DiminishedImage, in_TexCoordsFlipped + vec2(0.01f, 0.0f), 1.0f).a;
    float s3 = textureLod(g_DiminishedImage, in_TexCoordsFlipped + vec2(0.0f, -0.01f), 1.0f).a;
    float s4 = textureLod(g_DiminishedImage, in_TexCoordsFlipped + vec2(0.0f, 0.01f), 1.0f).a;

    vec3 Color;
    if (Diminished.a < 0.5f && s1 + s2 + s3 + s4 == 0.0f)
    {
        Color = Background.rgb;
    }
    else
    {
        Color = Diminished.rgb + Membrane.rgb;
    }

    out_Color = vec4(Color, 1.0f);
}

#endif // __INCLUDE_FS_OUTLINE_GLSL__