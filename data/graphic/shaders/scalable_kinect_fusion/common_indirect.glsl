
////////////////////////////////////////////////////////////////////////
// Structs for indirect buffers
////////////////////////////////////////////////////////////////////////

struct SIndirect
{
    uint m_Count;
    uint m_InstanceCount;
    uint m_FirstIndex;
    uint m_BaseIndex;
};

struct SIndexedIndirect
{
    uint m_IndexCount;
    uint m_InstanceCount;
    uint m_FirstIndex;
    uint m_BaseVertex;
    uint m_BaseInstance;
};

struct SComputeIndirect
{
    uint m_Count;
    uint m_InstanceCount;
    uint m_FirstIndex;
    uint m_BaseIndex;
};

struct SIndirectBuffers
{
    SIndirect m_Draw;
    SIndexedIndirect m_Indexed;
    SComputeIndirect m_Compute;
};