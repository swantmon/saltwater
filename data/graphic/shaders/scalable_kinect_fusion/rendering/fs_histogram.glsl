
#ifndef __INCLUDE_FS_OUTLINE_GLSL__
#define __INCLUDE_FS_OUTLINE_GLSL__

layout (binding = 0, r32i) uniform iimage2D fs_Histogram;

layout(location = 0) in vec2 in_TexCoords;

layout(location = 0) out vec4 out_Color;

void main()
{
    ivec2 Coords = ivec2(in_TexCoords * imageSize(fs_Histogram));

    out_Color = vec4(imageLoad(fs_Histogram, Coords).r / 100.0f, 0.0f, 0.0f, 1.0);
}

#endif // __INCLUDE_FS_OUTLINE_GLSL__