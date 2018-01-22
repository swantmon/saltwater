
#ifndef __INCLUDE_FS_OUTLINE_GLSL__
#define __INCLUDE_FS_OUTLINE_GLSL__

layout (binding = 0, r32i) uniform iimage2D cs_Histogram;

layout(location = 0) out vec4 out_Color;

void main()
{
    out_Color = vec4(1.0f, 0.0f, 0.0f, 1.0);
}

#endif // __INCLUDE_FS_OUTLINE_GLSL__