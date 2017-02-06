
#ifndef __INCLUDE_CS_KINECT_MIRROR_DEPTH_GLSL__
#define __INCLUDE_CS_KINECT_MIRROR_DEPTH_GLSL__

// -----------------------------------------------------------------------------
// Constants
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Input from engine
// -----------------------------------------------------------------------------

layout (binding = 0, r16ui) uniform uimage2D cs_Depth;

// -------------------------------------------------------------------------------------
// Functions
// -------------------------------------------------------------------------------------
layout (local_size_x = TILE_SIZE2D, local_size_y = TILE_SIZE2D, local_size_z = 1) in;
void main()
{
	const uint x = gl_GlobalInvocationID.x;
	const uint y = gl_GlobalInvocationID.y;
	
	if (gl_GlobalInvocationID.x < DEPTH_IMAGE_WIDTH / 2)
	{
		int Left = int(imageLoad(cs_Depth, ivec2(x, y)).x);
		int Right = int(imageLoad(cs_Depth, ivec2(DEPTH_IMAGE_WIDTH - x, y)).x);
		
	    imageStore(cs_Depth, ivec2(DEPTH_IMAGE_WIDTH - x, y), ivec4(Left));
        imageStore(cs_Depth, ivec2(x, y), ivec4(Right));
	}
}

#endif // __INCLUDE_CS_KINECT_MIRROR_DEPTH_GLSL__