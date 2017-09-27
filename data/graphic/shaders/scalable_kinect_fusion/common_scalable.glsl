
#ifndef __INCLUDE_SCALABLE_COMMON_GLSL__
#define __INCLUDE_SCALABLE_COMMON_GLSL__

struct SVolumePoolItem
{
    ivec3 m_Offset;
    int m_PoolIndex;
    bool m_NearSurface;
};

struct SGridPoolItem
{
    int m_PoolIndex;
    bool m_NearSurface;
};

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