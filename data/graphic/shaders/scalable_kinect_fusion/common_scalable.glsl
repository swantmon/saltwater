
#ifndef __INCLUDE_SCALABLE_COMMON_GLSL__
#define __INCLUDE_SCALABLE_COMMON_GLSL__

vec3 IndextoOffset(int Index, int Resolution)
{
    int z = Index / (Resolution * Resolution);
    Index -= (z * Resolution * Resolution);
    int y = Index / Resolution;
    int x = Index % Resolution;
    return vec3(x, y, z);
}

#endif // __INCLUDE_SCALABLE_COMMON_GLSL__