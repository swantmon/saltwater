
#ifndef __INCLUDE_COMMON_PLANE_DETECTION_GLSL__
#define __INCLUDE_COMMON_PLANE_DETECTION_GLSL__

vec2 NormalToSpherical(vec3 Normal)
{
    float Azimuth = atan(Normal.y, Normal.x);
    float Inclination = acos(Normal.z);

    return vec2(Azimuth, Inclination);
}

vec3 SphericalToNormal(vec2 Spherical)
{
    return vec3(0.0f);
}

#endif // __INCLUDE_COMMON_PLANE_DETECTION_GLSL__