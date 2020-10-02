
#ifndef __INCLUDE_FS_OUTLINE_GLSL__
#define __INCLUDE_FS_OUTLINE_GLSL__

layout(binding = 0) uniform sampler2D g_BackgroundImage;

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
    out_Color = out_Color = texture(g_BackgroundImage, g_Flipped > 0.5f ? in_TexCoordsFlipped : in_TexCoords);
    //out_Color = vec4(1.0f, 0.0f, 0.0f, 1.0f);
}

#endif // __INCLUDE_FS_OUTLINE_GLSL__