#ifndef __INCLUDE_CS_YUV_TO_RGB_GLSL__
#define __INCLUDE_CS_YUV_TO_RGB_GLSL__

layout (binding = 0, rgba8) uniform image2D cs_Diminished;
layout (binding = 1, rgba8) uniform image2D cs_Background;
layout (binding = 2, rgba16f) uniform image2D cs_MembranePatches;
layout (binding = 3, rgba16f) uniform image2D cs_MembraneBorders;

layout(std430, binding = 0) buffer Indirect
{
    uint g_Count;
    uint g_Y;
    uint g_Z;
    uint g_Unused;
};

layout(std430, binding = 1) buffer BorderPatches
{
    vec4 g_PatchPositions[MAX_BORDER_PATCH_COUNT];
    vec4 g_PatchColors[MAX_BORDER_PATCH_COUNT];
};

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

        if (gl_LocalInvocationIndex == 0)
        {
            uint Index = atomicAdd(g_Count, 1);
            g_PatchPositions[Index] = vec4(gl_WorkGroupID.xy, 0.0f, 0.0f);
        }
    }
}

#endif //__INCLUDE_CS_YUV_TO_RGB_GLSL__