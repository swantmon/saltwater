
#ifndef __INCLUDE_CS_PLANE_CANDIDATES_GLSL__
#define __INCLUDE_CS_PLANE_CANDIDATES_GLSL__

#include "slam/scalable_kinect_fusion/plane_detection/common_plane_detection.glsl"

layout(std430, binding = 0) buffer PlaneCountBuffer
{
    uint g_IndirectX;
    uint g_IndirectY;
    uint g_IndirectZ;
    uint g_MaxPlaneCount;
};

layout(std430, binding = 1) buffer PlaneBuffer
{
    vec4 g_Planes[];
};

// -----------------------------------------------------------------------------
// Input from engine
// -----------------------------------------------------------------------------

layout (binding = 0, r32i) uniform iimage2D cs_Histogram;
//layout (binding = 1, MAP_TEXTURE_FORMAT) uniform image2D cs_VertexMap;
//layout (binding = 2, MAP_TEXTURE_FORMAT) uniform image2D cs_NormalMap;

// -------------------------------------------------------------------------------------
// Functions
// -------------------------------------------------------------------------------------

layout (local_size_x = TILE_SIZE2D, local_size_y = TILE_SIZE2D, local_size_z = 1) in;
void main()
{    
    const int x = int(gl_GlobalInvocationID.x);
    const int y = int(gl_GlobalInvocationID.y);
    
    int Count = imageLoad(cs_Histogram, ivec2(x, y)).x;
    
    if (Count > 100)
    {
        bool IsHotspot = true;

        for (int i = -1; i <= 1; ++ i)
        {
            for (int j = -1; j <= 1; ++ j)
            {
                if (imageLoad(cs_Histogram, ivec2(x + i, y + j)).x > Count)
                {
                    IsHotspot = false;
                }
            }
        }

        if (IsHotspot)
        {
            uint PlaneIndex = atomicAdd(g_IndirectZ, 1);

            if (PlaneIndex < g_MaxPlaneCount)
            {
                vec2 Spherical = BinToSpherical(ivec2(x, y), g_AzimuthBinCount, g_InclinationBinCount);
                vec3 Normal = SphericalToCartesian(Spherical);
                g_Planes[PlaneIndex] = vec4(Normal, 0.0f);
            }
        }
    }
}

#endif // __INCLUDE_CS_PLANE_CANDIDATES_GLSL__