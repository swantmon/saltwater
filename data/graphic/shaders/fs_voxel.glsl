
#ifndef __INCLUDE_FS_VOXEL_GLSL__
#define __INCLUDE_FS_VOXEL_GLSL__

layout(location = 0) in float in_Color;

// -----------------------------------------------------------------------------
// Output to fragment
// -----------------------------------------------------------------------------
layout(location = 0) out vec4 out_Color;

void main(void)
{
	if (in_Color < 0.5f)
	{
		discard;
	}
    out_Color = vec4(0.0, in_Color, 0.0, 1.0f);
}

#endif // __INCLUDE_FS_VOXEL_GLSL__