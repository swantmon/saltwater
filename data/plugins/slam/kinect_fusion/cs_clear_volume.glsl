
#ifndef __INCLUDE_CS_KINECT_SPHERE_VOLUME_GLSL__
#define __INCLUDE_CS_KINECT_SPHERE_VOLUME_GLSL__

// -----------------------------------------------------------------------------
// Constants
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Input from engine
// -----------------------------------------------------------------------------

layout (binding = 0, rg16f) writeonly uniform image3D cs_TSDFVolume;

#ifdef CAPTURE_COLOR
layout (binding = 1, rgba8) writeonly uniform image3D cs_ColorVolume;
#endif

// -------------------------------------------------------------------------------------
// Functions
// -------------------------------------------------------------------------------------

layout (local_size_x = TILE_SIZE3D, local_size_y = TILE_SIZE3D, local_size_z = TILE_SIZE3D) in;
void main()
{
	imageStore(cs_TSDFVolume, ivec3(gl_GlobalInvocationID), vec4(0));
#ifdef CAPTURE_COLOR
	imageStore(cs_ColorVolume, ivec3(gl_GlobalInvocationID), ivec4(0));
#endif
}

#endif // __INCLUDE_CS_KINECT_SPHERE_VOLUME_GLSL__