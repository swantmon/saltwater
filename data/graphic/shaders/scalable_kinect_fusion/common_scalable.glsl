
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
    uint g_TSDFPool[];
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

#ifdef CAPTURE_COLOR

uint PackVoxel(float TSDF, float Weight, vec3 Color)
{
    uvec3 RGB565 = uvec3(Color * 255.0f);
    RGB565.r = ((RGB565.r >> 3) & 0x1F);
    RGB565.g = ((RGB565.g >> 2) & 0x3F) << 5;
    RGB565.b = ((RGB565.b >> 3) & 0x1F) << 11;

    uint PackedColor = (RGB565.r | RGB565.g | RGB565.b) << 16;

    uint PackedWeight = (uint(Weight) & 0x1F) << 11;
    uint PackedTSDF = uint((TSDF * 0.5f + 0.5f) * 2048.0f) & 0x7FF;

    return PackedColor | PackedWeight | PackedTSDF;
}

vec2 UnpackVoxel(uint Voxel, out vec3 Color)
{
    uint PackedTSDF = Voxel & 0x7FF;
    uint PackedWeight = (Voxel >> 11) & 0x1F;

    vec2 Result;
    Result.x = (float(PackedTSDF) / 2048.0f) * 2.0f - 1.0f;
    Result.y = float(PackedWeight);

    Voxel = Voxel >> 16;

    uvec3 RGB565;
    RGB565.r = (Voxel & 0x1F) << 3;
    RGB565.g = ((Voxel >> 5) & 0x3F) << 2;
    RGB565.b = ((Voxel >> 11) & 0x1F) << 3;

    Color = RGB565 / 255.0f;

    return Result;
}

vec2 UnpackVoxel(uint Voxel)
{
    uint PackedTSDF = Voxel & 0x7FF;
    uint PackedWeight = (Voxel >> 11) & 0x1F;

    vec2 Result;
    Result.x = (float(PackedTSDF) / 2048.0f) * 2.0f - 1.0f;
    Result.y = float(PackedWeight);

    return Result;
}

#else

uint PackVoxel(float TSDF, float Weight)
{
    return packSnorm2x16(vec2(TSDF, Weight / MAX_INTEGRATION_WEIGHT));
}

vec2 UnpackVoxel(uint Voxel)
{
    vec2 Data = unpackSnorm2x16(Voxel);

    Data.y *= MAX_INTEGRATION_WEIGHT;

    return Data;
}

#endif // CAPTURE_COLOR

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