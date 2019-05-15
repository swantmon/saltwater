
#ifndef __INCLUDE_FS_OUTLINE_GLSL__
#define __INCLUDE_FS_OUTLINE_GLSL__

layout(std140, binding = 1) uniform PerDrawCallData
{
    mat4 g_WorldMatrix;
	vec4 g_Color;
};

layout (binding = 0, rgba8) uniform image2D cs_Texture;

layout(location = 0) in vec3 in_WSPosition;

void main()
{
    imageStore(cs_Texture, ivec2(gl_FragCoord.xy), vec4(0.5f));
}

#endif // __INCLUDE_FS_OUTLINE_GLSL__