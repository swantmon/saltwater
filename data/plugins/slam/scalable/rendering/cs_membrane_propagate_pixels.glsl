#ifndef __INCLUDE_CS_YUV_TO_RGB_GLSL__
#define __INCLUDE_CS_YUV_TO_RGB_GLSL__

layout (binding = 0, rgba8) uniform image2D cs_Diminished;
layout (binding = 1, rgba8) uniform image2D cs_Background;
layout (binding = 2, rgba16f) uniform image2D cs_MembranePatches;
layout (binding = 3, rgba16f) uniform image2D cs_MembraneBorders;
layout (binding = 4, rgba16f) uniform image2D cs_MembraneFinale;

layout(std430, binding = 0) buffer BorderPatches
{
    int g_Count;
    vec4 g_PatchPositions[MAX_BORDER_PATCH_COUNT];
    vec4 g_PatchColors[MAX_BORDER_PATCH_COUNT];
};

layout (local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main()
{
    const ivec2 Coords = ivec2(gl_GlobalInvocationID.xy);

    vec3 Color = imageLoad(cs_MembranePatches, Coords).rgb;
    vec3 GridColor = imageLoad(cs_MembraneBorders, Coords - (Coords % PATCH_SIZE)).rgb;
    
    if (Color.r != 0.0f && Color.g != 0.0f && Color.b != 0.0f)
    {
        imageStore(cs_MembraneFinale, Coords, vec4(GridColor, 1.0f));
    }
}

#endif //__INCLUDE_CS_YUV_TO_RGB_GLSL__