
#ifndef __INCLUDE_CS_VOXEL_GLSL__
#define __INCLUDE_CS_VOXEL_GLSL__

// -----------------------------------------------------------------------------
// Defines
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Input from engine
// -----------------------------------------------------------------------------

layout (binding = 0, r16) writeonly uniform image3D cs_OutputTexture;

// -------------------------------------------------------------------------------------
// Functions
// -------------------------------------------------------------------------------------
layout (local_size_x = TILE_SIZE, local_size_y = TILE_SIZE, local_size_z = TILE_SIZE) in;
void main()
{
	float Distance = length(gl_GlobalInvocationID - vec3(CUBE_WIDTH / 2.0));
	
    imageStore(cs_OutputTexture, ivec3(gl_GlobalInvocationID), vec4(Distance > CUBE_WIDTH / 2.0 ? 0.0 : 1.0));
	//imageStore(cs_OutputTexture, ivec3(gl_GlobalInvocationID), vec4(1.0, 1.0, 1.0, 1.0));
}

#endif // __INCLUDE_CS_VOXEL_GLSL__