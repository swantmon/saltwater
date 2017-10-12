
#ifndef __INCLUDE_CS_GATHER_GATHER_FULL_GLSL__
#define __INCLUDE_CS_GATHER_GATHER_FULL_GLSL__

#include "scalable_kinect_fusion/common_scalable.glsl"
#include "scalable_kinect_fusion/common_indirect.glsl"

// -----------------------------------------------------------------------------
// Buffers
// -----------------------------------------------------------------------------

layout(binding = 1, r8ui) uniform uimage3D cs_Volume;

// -----------------------------------------------------------------------------
// Compute Shader
// -----------------------------------------------------------------------------

layout (local_size_x = 8, local_size_y = 8, local_size_z = 8) in;
void main()
{
    imageStore(cs_Volume, ivec3(gl_GlobalInvocationID), uvec4(0));
}

#endif // __INCLUDE_CS_GATHER_GATHER_FULL_GLSL__