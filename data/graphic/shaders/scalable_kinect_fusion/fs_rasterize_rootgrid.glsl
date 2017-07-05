
#ifndef __INCLUDE_FS_RASTERIZE_ROOTGRID_GLSL__
#define __INCLUDE_FS_RASTERIZE_ROOTGRID_GLSL__

layout(row_major, std140, binding = 2) uniform UBOOffset
{
    vec3 g_Offset;
    int g_Index;
};

layout(row_major, std140, binding = 3) uniform UBOHierarchy
{
    float g_HierarchyResolutions[HIERARCHY_LEVELS];
};

layout(std430, binding = 0) buffer ICPBuffer
{
    uint g_Counters[];
};

layout(binding = 0, r16ui) readonly uniform uimage2D cs_Depth;

layout(location = 0) out vec4 out_Color;

void main()
{
    atomicAdd(g_Counters[g_Index], 1);
    out_Color = vec4(imageLoad(cs_Depth, ivec2(gl_FragCoord.xy)));
}

#endif // __INCLUDE_FS_RASTERIZE_ROOTGRID_GLSL__
