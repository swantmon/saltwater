
#ifndef __INCLUDE_FS_OUTLINE_GLSL__
#define __INCLUDE_FS_OUTLINE_GLSL__

layout(std140, binding = 1) uniform PerDrawCallData
{
    mat4 g_WorldMatrix;
    vec4 g_Scale;
};

layout(binding = 0) uniform sampler2D PlaneTexture;

layout(location = 0) in vec2 in_TexCoords;

layout(location = 0) out vec4 out_Color;

void main()
{
    vec2 Tex = abs(in_TexCoords * 2.0f - 1.0f);

    if (Tex.x > (1.0f / g_Scale.y) || Tex.y > (1.0f / g_Scale.y))
    {
        out_Color = vec4(texture(PlaneTexture, in_TexCoords).rgb, 0.0f);
    }
    else
    {
        out_Color = vec4(texture(PlaneTexture, in_TexCoords).rgb, 1.0f);
    }
}

#endif // __INCLUDE_FS_OUTLINE_GLSL__