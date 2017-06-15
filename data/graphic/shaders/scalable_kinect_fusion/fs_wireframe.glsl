
#ifndef __INCLUDE_FS_WIREFRAME_GLSL__
#define __INCLUDE_FS_WIREFRAME_GLSL__

layout(location = 0) out vec4 out_Color;

void main()
{
    out_Color = vec4(1.0f, 0.0f, 1.0f, 1.0f);
}

#endif // __INCLUDE_FS_WIREFRAME_GLSL__