#ifndef __INCLUDE_CS_YUV_TO_RGB_GLSL__
#define __INCLUDE_CS_YUV_TO_RGB_GLSL__

layout (binding = 0, rgba8) uniform image2D cs_Diminished;
layout (binding = 1, rgba8) uniform image2D cs_Background;
layout (binding = 2, rgba16f) uniform image2D cs_MembranePatches;
layout (binding = 3, rgba16f) uniform image2D cs_MembraneBorders;

layout(std430, binding = 0) buffer BorderPatches
{
    int g_Count;
    vec4 g_PatchPositions[MAX_BORDER_PATCH_COUNT];
    vec4 g_PatchColors[MAX_BORDER_PATCH_COUNT];
};

shared float g_WeightSum[MAX_BORDER_PATCH_COUNT];
shared vec3 g_ColorSum[MAX_BORDER_PATCH_COUNT];
shared int g_Valid[MAX_BORDER_PATCH_COUNT];

layout (local_size_x = MAX_BORDER_PATCH_COUNT, local_size_y = 1, local_size_z = 1) in;
void main()
{
    bool IsPatch = imageLoad(cs_MembranePatches, ivec2(gl_WorkGroupID * PATCH_SIZE)).x > 0.5f;

    if (!IsPatch)
    {
        return;
    }

    bool IsBorder = gl_LocalInvocationIndex < g_Count;

    vec2 PatchPosition = vec2(gl_WorkGroupID.xy + 0.5f) * PATCH_SIZE;

    vec2 BorderPosition = vec2(0.0f);
    vec3 BorderColor = vec3(0.0f);

    if (IsBorder)
    {
        vec2 BorderPosition = g_PatchPositions[gl_LocalInvocationIndex].xy;
        vec3 BorderColor = g_PatchColors[gl_LocalInvocationIndex].rgb;
    }

    float d = distance(PatchPosition, BorderPosition);

    float Weight = 1.0f / (d * d * d);

    g_WeightSum[gl_LocalInvocationIndex] = Weight;
    g_ColorSum[gl_LocalInvocationIndex] = Weight * BorderColor;
    g_Valid[gl_LocalInvocationIndex] = IsBorder ? 1 : 0;

    imageStore(cs_MembranePatches, ivec2(gl_WorkGroupID * PATCH_SIZE), vec4(g_Count, 1.0f, 0.0f, 1.0f));
}

#endif //__INCLUDE_CS_YUV_TO_RGB_GLSL__