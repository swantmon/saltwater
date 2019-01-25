#ifndef __INCLUDE_CS_CLEAR_ATOMIC_IMAGE_GLSL__
#define __INCLUDE_CS_CLEAR_ATOMIC_IMAGE_GLSL__

// -----------------------------------------------------------------------------
// Input from engine
// -----------------------------------------------------------------------------

layout(std430, binding = 0) buffer AtomicBuffer
{
    uint g_Counters[];
};

// -------------------------------------------------------------------------------------
// Functions
// -------------------------------------------------------------------------------------
layout (local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main()
{
    g_Counters[gl_GlobalInvocationID.x] = 0;
}

#endif // __INCLUDE_CS_CLEAR_ATOMIC_IMAGE_GLSL__