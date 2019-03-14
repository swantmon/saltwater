#ifndef __INCLUDE_CS_YUV_TO_RGB_GLSL__
#define __INCLUDE_CS_YUV_TO_RGB_GLSL__

layout (binding = 0, rgba8) uniform image2D cs_Diminished;
layout (binding = 1, rgba8) uniform image2D cs_Background;
layout (binding = 2, rgba8) uniform image2D cs_MembranePatches;
layout (binding = 3, rgba8) uniform image2D cs_MembraneBorders;

layout(std430, binding = 0) buffer BorderPatches
{
    int g_Count;
    vec4 g_PatchPositions[MAX_BORDER_PATCH_COUNT];
    vec4 g_PatchColors[MAX_BORDER_PATCH_COUNT];
};

layout (local_size_x = TILE_SIZE_2D, local_size_y = TILE_SIZE_2D, local_size_z = 1) in;
void main()
{
    ivec2 ImageSize = imageSize(cs_Background);
    ivec2 MembraneSize = imageSize(cs_MembranePatches);

    vec2 PatchPosition = g_PatchPositions[gl_WorkGroupID.x].xy * 16;
    const ivec2 MembraneCoords = ivec2(PatchPosition + gl_LocalInvocationID.xy);

    imageStore(cs_MembranePatches, MembraneCoords, vec4(1.0f, 0.0f, 0.0f, 1.0f));

    const ivec2 ImageCoords = ivec2(MembraneCoords / vec2(MembraneSize) * vec2(ImageSize));
    const ivec2 InverseCoords = ivec2(ImageCoords.x, MembraneSize.y - ImageCoords.y);

    vec4 Background = imageLoad(cs_Background, InverseCoords);
    vec4 Diminished = imageLoad(cs_Diminished, ImageCoords);
    
}

#endif //__INCLUDE_CS_YUV_TO_RGB_GLSL__