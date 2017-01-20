
#ifndef __INCLUDE_CS_KINECT_SPHERE_VOLUME_GLSL__
#define __INCLUDE_CS_KINECT_SPHERE_VOLUME_GLSL__

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

layout (local_size_x = TILE_SIZE3D, local_size_y = TILE_SIZE3D, local_size_z = TILE_SIZE3D) in;
void main()
{
	vec3 ImageSize = vec3(imageSize(cs_Volume));
	vec3 Position = vec3(gl_GlobalInvocationID);
	
	float Length = length(Position - ImageSize / 2.0);
	
	uint Color = Length < (ImageSize.x / 2.0) ? 1 : 0;
	
	imageStore(cs_Volume, ivec3(gl_GlobalInvocationID), ivec4(Color));
}

#endif // __INCLUDE_CS_KINECT_SPHERE_VOLUME_GLSL__