
#ifndef __INCLUDE_FS_KINECT_VISUALIZE_VOLUME_GLSL__
#define __INCLUDE_FS_KINECT_VISUALIZE_VOLUME_GLSL__

// -----------------------------------------------------------------------------
// Output to fragment
// -----------------------------------------------------------------------------

layout(location = 0) in vec4 in_Color;
layout(location = 0) out vec4 out_Color;

void main(void)
{
	if (in_Color.r == 0.0)
	{
		discard;
	}
    out_Color = vec4(0.0, 1.0, 0.0, 1.0);
}

#endif // __INCLUDE_FS_KINECT_VISUALIZE_VOLUME_GLSL__