
#ifndef __INCLUDE_FS_OUTLINE_GLSL__
#define __INCLUDE_FS_OUTLINE_GLSL__

layout(std140, binding = 1) uniform PerDrawCallData
{
    mat4 g_WorldMatrix;
	vec4 g_Color;
};

layout (binding = 0, rgba8) uniform image2D cs_Diminished;

layout(location = 0) in vec2 in_UV;

layout(location = 0) out vec4 out_Color;

void main()
{
    out_Color = vec4(texture(PlaneTexture, in_UV).rgb, 1.0f);
}

#endif // __INCLUDE_FS_OUTLINE_GLSL__