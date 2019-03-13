
#ifndef __INCLUDE_FS_OUTLINE_GLSL__
#define __INCLUDE_FS_OUTLINE_GLSL__

layout(binding = 0) uniform sampler2D g_BackgroundImage;
layout(binding = 1) uniform sampler2D g_DiminishedImage;

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
    out_Color = texture(g_BackgroundImage, in_TexCoordsFlipped) + texture(g_DiminishedImage, in_TexCoords);
}

#endif // __INCLUDE_FS_OUTLINE_GLSL__