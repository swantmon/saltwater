
#ifndef __INCLUDE_FS_KINECT_VISUALIZE_VERTEX_MAP_GLSL__
#define __INCLUDE_FS_KINECT_VISUALIZE_VERTEX_MAP_GLSL__

layout(binding = 1, rgba32f) readonly uniform image2D fs_NormalMap;

layout(location = 0) in flat int IsValid;
layout(location = 1) in flat ivec2 TexCoords;

layout(location = 0) out vec4 out_Color;

void main(void)
{
	if(IsValid == 0)
	{
		discard;
	}
    
    out_Color = vec4(imageLoad(fs_NormalMap, TexCoords).xyz * 0.5f + 0.5f, 1.0);
}

#endif // __INCLUDE_FS_KINECT_VISUALIZE_VERTEX_MAP_GLSL__