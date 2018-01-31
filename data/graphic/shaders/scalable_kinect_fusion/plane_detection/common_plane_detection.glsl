
#ifndef __INCLUDE_COMMON_PLANE_DETECTION_GLSL__
#define __INCLUDE_COMMON_PLANE_DETECTION_GLSL__

layout(row_major, std140, binding = 0) uniform HistogramSizes
{
    mat4 g_PoseMatrix;
    mat4 g_InvPoseMatrix;
    int g_AzimuthBinCount;
    int g_InclinationBinCount;
    int g_Unused0;
    int g_Unused1;
};

const float g_Tau = 6.28318530718f;
const float g_Pi = g_Tau * 0.5f;
const int g_KernelSize = 3;

// Functions to convert between cartesian and spherical coordinates
// r in spherical is always 1 when the normal is normalized

vec2 CartesianToSpherical(vec3 Normal)
{
    float Azimuth = atan(Normal.y, Normal.x);
    float Inclination = acos(Normal.z);

    return vec2(Azimuth, Inclination);
}

vec3 SphericalToCartesian(vec2 Spherical)
{
    float Azimuth = Spherical.x;
    float Inclination = Spherical.y;

    vec3 Normal;

    Normal.x = sin(Inclination) * cos(Azimuth);
    Normal.y = sin(Inclination) * sin(Azimuth);
    Normal.z = cos(Inclination);

    return Normal;
}

ivec2 SphericalToBin(float Azimuth, float Inclination, int AzimuthBinCount, int InclinationBinCount)
{
    int AzimuthBin = int((Azimuth / g_Tau + 0.5f) * AzimuthBinCount);
    int InclinationBin = int((Inclination / g_Pi) * InclinationBinCount);

    return ivec2(AzimuthBin, InclinationBin);
}

ivec2 SphericalToBin(vec2 Spherical, int AzimuthBinCount, int InclinationBinCount)
{
    return SphericalToBin(Spherical.x, Spherical.y, AzimuthBinCount, InclinationBinCount);
}

vec2 BinToSpherical(ivec2 Bin, int AzimuthBinCount, int InclinationBinCount)
{
    float Azimuth = (float(Bin.x) / AzimuthBinCount - 0.5f) * g_Tau;
    float Inclination = (float(Bin.y) / InclinationBinCount) * g_Pi;

    return vec2(Azimuth, Inclination);
}

ivec2 CartesianToBin(vec3 Normal, int AzimuthBinCount, int InclinationBinCount)
{
    return SphericalToBin(CartesianToSpherical(Normal), AzimuthBinCount, InclinationBinCount);
}

vec3 BinToCartesian(ivec2 Bin, int AzimuthBinCount, int InclinationBinCount)
{
    return SphericalToCartesian(BinToSpherical(Bin, AzimuthBinCount, InclinationBinCount));
}

int PlaneDistanceToBin(float D, int BinCount)
{
    return int(((D / 10.0f) + 0.5f) * BinCount);
}

float BinToPlaneDistance(int Bin, int BinCount)
{
    return (float(Bin) / BinCount - 0.5f) * 10.0f;
}


#endif // __INCLUDE_COMMON_PLANE_DETECTION_GLSL__