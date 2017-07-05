#ifndef __INCLUDE_CS_CLEAR_ATOMIC_IMAGE_GLSL__
#define __INCLUDE_CS_CLEAR_ATOMIC_IMAGE_GLSL__

// -----------------------------------------------------------------------------
// Input from engine
// -----------------------------------------------------------------------------

layout(binding = 0, r32ui) uniform uimage2D cs_Counter;

// -------------------------------------------------------------------------------------
// Functions
// -------------------------------------------------------------------------------------
layout (local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main()
{
    imageStore(cs_Counter, ivec2(gl_GlobalInvocationID.xy), uvec4(0));
}

#endif // __INCLUDE_CS_CLEAR_ATOMIC_IMAGE_GLSL__