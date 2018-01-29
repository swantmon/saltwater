
#ifndef __INCLUDE_CS_PLANE_DETECTION_GLSL__
#define __INCLUDE_CS_PLANE_DETECTION_GLSL__

layout(row_major, std140, binding = 0) uniform HistogramSizes
{
    mat4 g_PoseMatrix;
    int g_AzimuthBinCount;
    int g_InclinationBinCount;
    int g_Unused0;
    int g_Unused1;
};

const float g_Tau = 6.28318530718f;
const float g_Pi = g_Tau * 0.5f;
const int g_KernelSize = 3;

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
    
    vec3 Normal = vec3(0.0f);
    int Count = 0;
    
    for (int i = -g_KernelSize; i <= g_KernelSize; ++ i)
    {
        for (int j = -g_KernelSize; j <= g_KernelSize; ++ j)
        {
            vec3 Sample = imageLoad(cs_NormalMap, ivec2(x + i, y + j)).xyz;
            
            if (Sample.x != 0.0f)
            {
                ++ Count;
                Normal += Sample;
            }
        }
    }
    
    if (Count > 0)
    {
        Normal /= Count;
    
        Normal = mat3(g_PoseMatrix) * Normal;

        float Azimuth = atan(Normal.y, Normal.x);
        float Inclination = acos(Normal.z);
        
        int AzimuthBin = int((Azimuth / g_Tau + 0.5f) * g_AzimuthBinCount);
        int InclinationBinY = int((Inclination / g_Pi) * g_InclinationBinCount);
    
        imageAtomicAdd(cs_Histogram, ivec2(AzimuthBin, InclinationBinY), 1);
    }
}

#endif // __INCLUDE_CS_PLANE_DETECTION_GLSL__