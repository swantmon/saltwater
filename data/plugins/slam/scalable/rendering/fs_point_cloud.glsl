
#ifndef __INCLUDE_FS_POINT_CLOUD_GLSL__
#define __INCLUDE_FS_POINT_CLOUD_GLSL__

layout(location = 0) in vec3 in_Color;

layout(location = 0) out vec4 out_Color;

void main()
{
    out_Color = vec4(in_Color, 1.0f);
    gl_FragDepth = 0.0f;
}

#endif // __INCLUDE_FS_POINT_CLOUD_GLSL__