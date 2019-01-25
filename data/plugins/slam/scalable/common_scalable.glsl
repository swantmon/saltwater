
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

#ifdef CAPTURE_COLOR

struct STSDFPoolItem
{
    float m_TSDF;
    uint m_Color;
};

#else

struct STSDFPoolItem
{
    uint m_TSDF;
};

#endif

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

////////////////////////////////////////////////////////////////////////
// Pack and unpack voxels
////////////////////////////////////////////////////////////////////////

#ifdef CAPTURE_COLOR

STSDFPoolItem PackVoxel(float TSDF, float Weight, vec3 Color)
{
    STSDFPoolItem Voxel;
    Voxel.m_TSDF = TSDF;
    Voxel.m_Color = packUnorm4x8(vec4(Color, Weight / MAX_INTEGRATION_WEIGHT));

    return Voxel;
}

vec2 UnpackVoxel(STSDFPoolItem Voxel, out vec3 Color)
{
    vec4 ColorData = unpackUnorm4x8(Voxel.m_Color);

    Color = ColorData.rgb;

    return vec2(Voxel.m_TSDF, ColorData.a * MAX_INTEGRATION_WEIGHT);
}

vec2 UnpackVoxel(STSDFPoolItem Voxel)
{
    vec4 ColorData = unpackUnorm4x8(Voxel.m_Color);

    return vec2(Voxel.m_TSDF, ColorData.a * MAX_INTEGRATION_WEIGHT);
}

#else

STSDFPoolItem PackVoxel(float TSDF, float Weight)
{
    Weight /= MAX_INTEGRATION_WEIGHT;

    STSDFPoolItem Voxel;
    Voxel.m_TSDF = packSnorm2x16(vec2(TSDF, Weight));

    return Voxel;
}

vec2 UnpackVoxel(STSDFPoolItem Voxel)
{
    vec2 Data = unpackSnorm2x16(Voxel.m_TSDF);

    Data.y *= MAX_INTEGRATION_WEIGHT;

    return Data;
}

#endif

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