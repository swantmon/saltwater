
#ifndef __INCLUDE_CS_KINECT_BILATERAL_FILTER_GLSL__
#define __INCLUDE_CS_KINECT_BILATERAL_FILTER_GLSL__

// -----------------------------------------------------------------------------
// Constants
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Input from engine
// -----------------------------------------------------------------------------

layout (binding = 0, rg16ui) writeonly uniform uimage3D cs_Volume;

// -------------------------------------------------------------------------------------
// Functions
// -------------------------------------------------------------------------------------
layout (local_size_x = TILE_SIZE, local_size_y = TILE_SIZE, local_size_z = TILE_SIZE) in;
void main()
{
	imageStore(cs_Volume, ivec3(gl_GlobalInvocationID), uvec4(gl_GlobalInvocationID, 0));
}

#endif // __INCLUDE_CS_KINECT_BILATERAL_FILTER_GLSL__