
#ifndef __INCLUDE_CS_GATHER_COUNTERS_GLSL__
#define __INCLUDE_CS_GATHER_COUNTERS_GLSL__

// -----------------------------------------------------------------------------
// Buffers
// -----------------------------------------------------------------------------

struct SIndexedIndirect
{
    uint  m_IndexCount;
    uint  m_InstanceCount;
    uint  m_FirstIndex;
    uint  m_BaseVertex;
    uint  m_BaseInstance;
};

layout(std430, binding = 0) buffer AtomicBuffer
{
    uint g_Counters[];
};

layout(std430, binding = 1) buffer IndirectBuffer
{
    SIndexedIndirect g_Indirect;
};

layout(row_major, std140, binding = 0) uniform UBO
{
    int g_InstanceCount;
};

// -------------------------------------------------------------------------------------
// Compute Shader
// -------------------------------------------------------------------------------------
layout (local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main()
{
    if (g_Counters[gl_GlobalInvocationID.x] > 0)
    {
        atomicAdd(g_Indirect.m_InstanceCount, 1);
    }
}

#endif // __INCLUDE_CS_GATHER_COUNTERS_GLSL__