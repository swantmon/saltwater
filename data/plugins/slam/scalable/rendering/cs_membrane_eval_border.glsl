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
    ivec2 BackgroundSize = imageSize(cs_Background);
    ivec2 DiminishedSize = imageSize(cs_Diminished);
    ivec2 MembraneSize = imageSize(cs_MembranePatches);

    vec2 PatchPosition = g_PatchPositions[gl_WorkGroupID.x].xy * PATCH_SIZE;
    const ivec2 MembraneCoords = ivec2(PatchPosition + gl_LocalInvocationID.xy);

    imageStore(cs_MembranePatches, MembraneCoords, vec4(1.0f, 0.0f, 0.0f, 1.0f));

    const vec2 NormalizedCoords = MembraneCoords / vec2(MembraneSize);

    const ivec2 DiminishedCoords = ivec2(NormalizedCoords * vec2(DiminishedSize));
    const ivec2 BackgroundCoords = ivec2(vec2(NormalizedCoords.x, 1.0f - NormalizedCoords.y) * vec2(BackgroundSize));

    vec3 Diminished = imageLoad(cs_Diminished, DiminishedCoords).rgb;
    vec3 Background = imageLoad(cs_Background, BackgroundCoords).rgb;

    if(Diminished.x > 0.0f)
    {
        vec3 ColorDiff = Diminished - Background;
        imageStore(cs_MembraneBorders, MembraneCoords, vec4(ColorDiff, 1.0f));
    }
}

#endif //__INCLUDE_CS_YUV_TO_RGB_GLSL__