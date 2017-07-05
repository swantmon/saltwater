
#ifndef __INCLUDE_FS_RASTERIZE_ROOTGRID_GLSL__
#define __INCLUDE_FS_RASTERIZE_ROOTGRID_GLSL__

layout(row_major, std140, binding = 3) uniform UBOHierarchy
{
    float g_HierarchyResolutions[HIERARCHY_LEVELS];
};

layout(binding = 0, r16ui) readonly uniform uimage2D cs_Depth;

layout(location = 0) out vec4 out_Color;

void main()
{
    out_Color = vec4(imageLoad(cs_Depth, ivec2(gl_FragCoord.xy)));
}

#endif // __INCLUDE_FS_RASTERIZE_ROOTGRID_GLSL__
