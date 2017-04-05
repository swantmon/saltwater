
#ifndef __INCLUDE_CS_KINECT_MIRROR_COLOR_GLSL__
#define __INCLUDE_CS_KINECT_MIRROR_COLOR_GLSL__

// -----------------------------------------------------------------------------
// Constants
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Input from engine
// -----------------------------------------------------------------------------

layout (binding = 0, rgba8ui) uniform uimage2D cs_Color;

// -------------------------------------------------------------------------------------
// Functions
// -------------------------------------------------------------------------------------
layout (local_size_x = TILE_SIZE2D, local_size_y = TILE_SIZE2D, local_size_z = 1) in;
void main()
{
	const uint x = gl_GlobalInvocationID.x;
	const uint y = gl_GlobalInvocationID.y;
	
	if (gl_GlobalInvocationID.x < COLOR_IMAGE_WIDTH / 2)
	{
		uvec4 Left = imageLoad(cs_Color, ivec2(x, y));
		uvec4 Right = imageLoad(cs_Color, ivec2(COLOR_IMAGE_WIDTH - x, y));
		
	    imageStore(cs_Color, ivec2(COLOR_IMAGE_WIDTH - x, y), Left);
        imageStore(cs_Color, ivec2(x, y), Right);
	}
}

#endif // __INCLUDE_CS_KINECT_MIRROR_COLOR_GLSL__