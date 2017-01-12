
#ifndef __INCLUDE_FS_KINECT_GLSL__
#define __INCLUDE_FS_KINECT_GLSL__

layout(binding = 0, r16) readonly uniform image2D vs_DepthData;

// -----------------------------------------------------------------------------
// Output to fragment
// -----------------------------------------------------------------------------
layout(location = 0) in vec2 in_TexCoord;
layout(location = 0) out vec4 out_Color;

void main(void)
{
	ivec2 ImageSize = imageSize(vs_DepthData);
	float Depth = imageLoad(vs_DepthData, ivec2(in_TexCoord * ImageSize)).x;
	Depth *= 10.0;
    out_Color = vec4(Depth, Depth, Depth, 1.0f);
}

#endif // __INCLUDE_FS_KINECT_GLSL__