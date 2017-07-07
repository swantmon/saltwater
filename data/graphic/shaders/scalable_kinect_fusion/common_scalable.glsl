
#ifndef __INCLUDE_SCALABLE_COMMON_GLSL__
#define __INCLUDE_SCALABLE_COMMON_GLSL__

int KeyToIndex(ivec3 Key, int Size) {
    return (Key.z * Size * Size) + (Key.y * Size) + Key.x;
}

ivec3 IndexToKey(int Index, int Size) {
    int z = Index / (Size * Size);
    Index -= (z * Size * Size);
    int y = Index / Size;
    int x = Index % Size;
    return ivec3(x, y, z);
}

#endif // __INCLUDE_SCALABLE_COMMON_GLSL__