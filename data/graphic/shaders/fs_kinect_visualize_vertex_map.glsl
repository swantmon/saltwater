
#ifndef __INCLUDE_FS_KINECT_VISUALIZE_VERTEX_MAP_GLSL__
#define __INCLUDE_FS_KINECT_VISUALIZE_VERTEX_MAP_GLSL__

layout(location = 0) in flat int IsValid;

layout(location = 0) out vec4 out_Color;

void main(void)
{
	if(IsValid == 0)
	{
		discard;
	}
	
    out_Color = vec4(0.0, 1.0, 0.0, 1.0f);
}

#endif // __INCLUDE_FS_KINECT_VISUALIZE_VERTEX_MAP_GLSL__