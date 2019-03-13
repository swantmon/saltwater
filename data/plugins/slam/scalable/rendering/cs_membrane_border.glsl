#ifndef __INCLUDE_CS_YUV_TO_RGB_GLSL__
#define __INCLUDE_CS_YUV_TO_RGB_GLSL__

layout (binding = 0, rgba8) uniform image2D cs_MembranePatches;
layout (binding = 1, rgba8) uniform image2D cs_MembraneBorders;

layout (local_size_x = TILE_SIZE_2D, local_size_y = TILE_SIZE_2D, local_size_z = 1) in;
void main()
{
    const ivec2 Coords = ivec2(gl_GlobalInvocationID.xy);

    bool IsMarked = imageLoad(cs_MembranePatches, Coords).r > 0.5f;

    bvec4 IsNeighbour;
    IsNeighbour.x = imageLoad(cs_MembranePatches, Coords + ivec2(            0,  TILE_SIZE_2D)).r > 0.5f;
    IsNeighbour.y = imageLoad(cs_MembranePatches, Coords + ivec2(            0, -TILE_SIZE_2D)).r > 0.5f;
    IsNeighbour.z = imageLoad(cs_MembranePatches, Coords + ivec2( TILE_SIZE_2D,             0)).r > 0.5f;
    IsNeighbour.w = imageLoad(cs_MembranePatches, Coords + ivec2(-TILE_SIZE_2D,             0)).r > 0.5f;

    if (any(IsNeighbour) && !IsMarked)
    {
        imageStore(cs_MembraneBorders, Coords, vec4(1.0f, 0.0f, 0.0f, 0.0f));
    }
}

#endif //__INCLUDE_CS_YUV_TO_RGB_GLSL__