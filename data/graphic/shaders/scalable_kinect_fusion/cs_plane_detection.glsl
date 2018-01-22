
#ifndef __INCLUDE_CS_PLANE_DETECTION_GLSL__
#define __INCLUDE_CS_PLANE_DETECTION_GLSL__

layout(row_major, std140, binding = 0) uniform HistogramSizes
{
    int g_AzimuthBinCount;
    int g_InclinationBinCount;
    int g_Unused0;
    int g_Unused1;
};

layout(std430, binding = 0) buffer Histogram
{
    int g_Histogram[];
};

const float g_Tau = 6.28318530718f;
const float g_Pi = g_Tau * 0.5f;

// -----------------------------------------------------------------------------
// Input from engine
// -----------------------------------------------------------------------------

layout (binding = 0, MAP_TEXTURE_FORMAT) uniform image2D cs_VertexMap;
layout (binding = 1, MAP_TEXTURE_FORMAT) uniform image2D cs_NormalMap;

// -------------------------------------------------------------------------------------
// Functions
// -------------------------------------------------------------------------------------
layout (local_size_x = TILE_SIZE2D, local_size_y = TILE_SIZE2D, local_size_z = 1) in;
void main()
{
    const ivec2 ImageSize = imageSize(cs_VertexMap);
    
    const int x = int(gl_GlobalInvocationID.x);
    const int y = int(gl_GlobalInvocationID.y);
    
    vec3 Normal = imageLoad(cs_NormalMap, ivec2(x, y)).xyz;
    
    float Azimuth = atan(Normal.y, Normal.x);
    float Inclination = acos(Normal.z);
        
    int AzimuthBin = int((Azimuth / g_Tau + 0.5f) * g_AzimuthBinCount);
    int InclinationBinY = int(Inclination / g_Pi * g_InclinationBinCount);
    
    atomicAdd(g_Histogram[InclinationBinY * g_AzimuthBinCount + InclinationBinY], 1);
}

#endif // __INCLUDE_CS_PLANE_DETECTION_GLSL__