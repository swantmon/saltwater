
#ifndef __INCLUDE_CS_GATHER_GATHER_GLSL__
#define __INCLUDE_CS_GATHER_GATHER_GLSL__

// -----------------------------------------------------------------------------
// Buffers
// -----------------------------------------------------------------------------

layout(binding = 1, r8ui) uniform uimage3D cs_Volume;

// -----------------------------------------------------------------------------
// Compute Shader
// -----------------------------------------------------------------------------

layout (local_size_x = 16, local_size_y = 16, local_size_z = 1) in;
void main()
{
    imageStore(cs_Volume, ivec3(gl_GlobalInvocationID), uvec4(0));
}

#endif // __INCLUDE_CS_GATHER_GATHER_GLSL__