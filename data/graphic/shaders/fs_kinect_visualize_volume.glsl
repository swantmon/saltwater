
#ifndef __INCLUDE_FS_KINECT_VISUALIZE_VOLUME_GLSL__
#define __INCLUDE_FS_KINECT_VISUALIZE_VOLUME_GLSL__

#define SHORT_MAX 65535.0

// -----------------------------------------------------------------------------
// Output to fragment
// -----------------------------------------------------------------------------

layout(location = 0) in vec4 in_Color;
layout(location = 0) out vec4 out_Color;

void main(void)
{
    out_Color = vec4(in_Color);
}

#endif // __INCLUDE_FS_KINECT_VISUALIZE_VOLUME_GLSL__