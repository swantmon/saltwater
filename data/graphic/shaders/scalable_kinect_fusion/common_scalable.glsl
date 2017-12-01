
#ifndef __INCLUDE_SCALABLE_COMMON_GLSL__
#define __INCLUDE_SCALABLE_COMMON_GLSL__

////////////////////////////////////////////////////////////////////////
// Pool item structures
////////////////////////////////////////////////////////////////////////

struct SVolumePoolItem
{
    ivec3 m_Offset;
    bool m_NearSurface;
};

struct SGridPoolItem
{
    int m_PoolIndex;
    bool m_NearSurface;
};

////////////////////////////////////////////////////////////////////////
// Pools
////////////////////////////////////////////////////////////////////////

layout(std430, binding = 0) buffer RootVolumePool
{
    SVolumePoolItem g_RootVolumePool[];
};

layout(std430, binding = 1) buffer RootGridPool
{
    SGridPoolItem g_RootGridPool[];
};

layout(std430, binding = 2) buffer Level1Pool
{
    SGridPoolItem g_Level1GridPool[];
};

layout(std430, binding = 3) buffer TSDFPool
{
    uint g_TSDFPool[]; // two 16 bit floats packed with packSnorm2x16
};

layout(std430, binding = 4) buffer PoolItemCounts
{
    int g_RootGridPoolItemCount;
    int g_Level1GridPoolItemCount;
    int g_TSDFPoolItemCount;
};

layout(std430, binding = 5) buffer VolumeIndexBuffer
{
    int g_CurrentVolumeIndex;
};

layout(std430, binding = 6) buffer RootVolumePositionBuffer
{
    int g_RootVolumePositionBuffer[];
};

////////////////////////////////////////////////////////////////////////
// Some helper functions
////////////////////////////////////////////////////////////////////////

vec3 IndexToOffset(uint Index, int Resolution)
{
    uint z = Index / (Resolution * Resolution);
    Index -= (z * Resolution * Resolution);
    uint y = Index / Resolution;
    uint x = Index % Resolution;
    return vec3(x, y, z);
}

int OffsetToIndex(vec3 Offset, int Resolution)
{
    ivec3 iOffset = ivec3(Offset);
    return (iOffset.z * Resolution * Resolution) + (iOffset.y * Resolution) + iOffset.x;
}

#endif // __INCLUDE_SCALABLE_COMMON_GLSL__