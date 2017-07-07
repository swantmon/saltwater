
#ifndef __INCLUDE_CS_GATHER_COUNTERS_GLSL__
#define __INCLUDE_CS_GATHER_COUNTERS_GLSL__

// -----------------------------------------------------------------------------
// Buffers
// -----------------------------------------------------------------------------

struct SIndexedIndirect
{
    uint  m_Count;
    uint  m_PrimCount;
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
layout (local_size_x = TILE_SIZE2D, local_size_y = TILE_SIZE2D, local_size_z = 1) in;
void main()
{
    
}

#endif // __INCLUDE_CS_GATHER_COUNTERS_GLSL__