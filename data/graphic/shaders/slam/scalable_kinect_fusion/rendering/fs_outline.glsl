
#ifndef __INCLUDE_FS_OUTLINE_GLSL__
#define __INCLUDE_FS_OUTLINE_GLSL__

layout(std140, binding = 1) uniform PerDrawCallData
{
    mat4 g_WorldMatrix;
	vec4 g_Color;
};

layout(location = 0) out vec4 out_Color;

void main()
{
    out_Color = g_Color;
}

#endif // __INCLUDE_FS_OUTLINE_GLSL__