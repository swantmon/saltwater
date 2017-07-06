
#ifndef __INCLUDE_FS_RASTERIZE_ROOTGRID_GLSL__
#define __INCLUDE_FS_RASTERIZE_ROOTGRID_GLSL__

struct SInstanceData
{
    ivec3 m_Offset;
    int m_Index;
};

layout(row_major, std140, binding = 3) uniform UBOHierarchy
{
    float g_HierarchyResolutions[HIERARCHY_LEVELS];
};

layout(std430, binding = 0) buffer AtomicBuffer
{
    uint g_Counters[];
};

layout(std430, binding = 1) buffer InstanceBuffer
{
    SInstanceData g_InstanceData[];
};

layout(binding = 0, r16ui) readonly uniform uimage2D cs_Depth;

layout(location = 0) in flat int in_Index;

layout(location = 0) out vec4 out_Color;

void main()
{
    atomicAdd(g_Counters[in_Index], 1);
    out_Color = vec4(imageLoad(cs_Depth, ivec2(gl_FragCoord.xy)));
}

#endif // __INCLUDE_FS_RASTERIZE_ROOTGRID_GLSL__
