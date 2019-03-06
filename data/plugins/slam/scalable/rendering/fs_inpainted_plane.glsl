
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
    if (Tex.x > 0.5f || Tex.y > 0.5f)
    {
        discard;
    }
    out_Color = texture(PlaneTexture, in_TexCoords);
    //out_Color = vec4(1.0f, 0.0f, 0.0f, 1.0f);
}

#endif // __INCLUDE_FS_OUTLINE_GLSL__