
#ifndef __INCLUDE_COMMON_PLANE_DETECTION_GLSL__
#define __INCLUDE_COMMON_PLANE_DETECTION_GLSL__

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

#endif // __INCLUDE_COMMON_PLANE_DETECTION_GLSL__