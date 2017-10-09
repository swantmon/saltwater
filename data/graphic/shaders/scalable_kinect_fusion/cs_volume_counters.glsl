
#ifndef __INCLUDE_CS_VOLUME_COUNTERS_GLSL__
#define __INCLUDE_CS_VOLUME_COUNTERS_GLSL__

#include "scalable_kinect_fusion/common_indirect.glsl"

// -----------------------------------------------------------------------------
// Buffers
// -----------------------------------------------------------------------------

layout(std430, binding = 0) buffer AtomicBuffer
{
    uint g_Counters[];
};

layout(std430, binding = 1) buffer IndirectBuffer
{
    SIndirectBuffers g_Indirect;
};

layout(std430, binding = 2) buffer VolumeQueue
{
    uint g_VolumeID[];
};

// -------------------------------------------------------------------------------------
// Compute Shader
// -------------------------------------------------------------------------------------
layout (local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main()
{
    if (g_Counters[gl_GlobalInvocationID.x] > 100)
    {
        uint InstanceIndex = atomicAdd(g_Indirect.m_Indexed.m_InstanceCount, 1);
        g_VolumeID[InstanceIndex] = gl_GlobalInvocationID.x;
    }
}

#endif // __INCLUDE_CS_VOLUME_COUNTERS_GLSL__