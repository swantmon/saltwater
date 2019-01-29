
#ifndef __INCLUDE_FS_OUTLINE_GLSL__
#define __INCLUDE_FS_OUTLINE_GLSL__

layout(binding = 0) uniform sampler2D PlaneTexture;

layout(location = 0) in vec2 in_TexCoords;

layout(location = 0) out vec4 out_Color;

void main()
{
    out_Color = texture(PlaneTexture, in_TexCoords);
    //out_Color = vec4(1.0f, 0.0f, 0.0f, 1.0f);
}

#endif // __INCLUDE_FS_OUTLINE_GLSL__