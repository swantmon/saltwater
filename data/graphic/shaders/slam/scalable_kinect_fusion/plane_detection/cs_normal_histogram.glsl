
#ifndef __INCLUDE_CS_NORMAL_HISTOGRAM_GLSL__
#define __INCLUDE_CS_NORMAL_HISTOGRAM_GLSL__

#include "slam/scalable_kinect_fusion/plane_detection/common_plane_detection.glsl"

// -----------------------------------------------------------------------------
// Input from engine
// -----------------------------------------------------------------------------

layout (binding = 0, r32i) uniform iimage2D cs_Histogram;
layout (binding = 1, MAP_TEXTURE_FORMAT) uniform image2D cs_VertexMap;
layout (binding = 2, MAP_TEXTURE_FORMAT) uniform image2D cs_NormalMap;

// -------------------------------------------------------------------------------------
// Functions
// -------------------------------------------------------------------------------------

layout (local_size_x = TILE_SIZE2D, local_size_y = TILE_SIZE2D, local_size_z = 1) in;
void main()
{    
    const int x = int(gl_GlobalInvocationID.x);
    const int y = int(gl_GlobalInvocationID.y);

    ivec2 ImageSize = imageSize(cs_VertexMap);

    if (x >= ImageSize.x || y >= ImageSize.y)
    {
        return;
    }

    vec3 Normal = vec3(0.0f);
    int Count = 0;
    
    for (int i = -g_KernelSize; i <= g_KernelSize; ++ i)
    {
        for (int j = -g_KernelSize; j <= g_KernelSize; ++ j)
        {
            if (x + i < ImageSize.x && x + i >= 0 &&
                y + j < ImageSize.y && y + j >= 0)
            {
                vec3 Sample = mat3(g_InvPoseMatrix) * imageLoad(cs_NormalMap, ivec2(x + i, y + j)).xyz;

                if (Sample.x > -5.0f)
                {
                    ++ Count;
                    Normal += Sample;
                }
            }
        }
    }
    
    if (Count > 0)
    {
        Normal /= Count;
    
        //Normal = mat3(g_PoseMatrix) * Normal;

        vec2 Spherical = CartesianToSpherical(Normal);

        float Azimuth = Spherical.x;
        float Inclination = Spherical.y;
        
        ivec2 Bin = SphericalToBin(Azimuth, Inclination, g_AzimuthBinCount, g_InclinationBinCount);

        imageAtomicAdd(cs_Histogram, Bin, 1);
    }
}

#endif // __INCLUDE_CS_NORMAL_HISTOGRAM_GLSL__