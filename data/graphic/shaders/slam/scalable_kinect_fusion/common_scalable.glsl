
#ifndef __INCLUDE_SCALABLE_COMMON_GLSL__
#define __INCLUDE_SCALABLE_COMMON_GLSL__

////////////////////////////////////////////////////////////////////////
// Pool item structures
////////////////////////////////////////////////////////////////////////

struct SVolumePoolItem
{
    ivec3 m_Offset;
    int m_Weight;
};

struct SGridPoolItem
{
    int m_PoolIndex;
    int m_Weight;
};


struct STSDFPoolItem
{
    uint m_TSDF;
#ifdef CAPTURE_COLOR
    uint m_Color;
#endif
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
    STSDFPoolItem g_TSDFPool[];
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

layout(std430, binding = 8) buffer Garbage
{
    int g_GarbageCount;
    int g_GarbageList[];
};

////////////////////////////////////////////////////////////////////////
// Pack and unpack voxels
////////////////////////////////////////////////////////////////////////

STSDFPoolItem PackVoxel(float TSDF, float Weight)
{
    Weight /= MAX_INTEGRATION_WEIGHT;

    STSDFPoolItem Voxel;
    Voxel.m_TSDF = packSnorm2x16(vec2(TSDF, Weight));

#ifdef CAPTURE_COLOR
    Voxel.m_Color = packUnorm4x8(vec4(0.0f));
#endif

    return Voxel;
}

#ifdef CAPTURE_COLOR

STSDFPoolItem PackVoxel(float TSDF, float Weight, vec3 Color)
{
    Weight /= MAX_INTEGRATION_WEIGHT;

    STSDFPoolItem Voxel;
    Voxel.m_TSDF = packSnorm2x16(vec2(TSDF, Weight));
    Voxel.m_Color = packUnorm4x8(vec4(Color, 1.0f));

    return Voxel;
}

vec2 UnpackVoxel(STSDFPoolItem Voxel, out vec3 Color)
{
    vec2 Data = unpackSnorm2x16(Voxel.m_TSDF);

    Data.y *= MAX_INTEGRATION_WEIGHT;

    Color = unpackUnorm4x8(Voxel.m_Color).rgb;

    return Data;
}


#endif

vec2 UnpackVoxel(STSDFPoolItem Voxel)
{
    vec2 Data = unpackSnorm2x16(Voxel.m_TSDF);

    Data.y *= MAX_INTEGRATION_WEIGHT;

    return Data;
}

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