
#ifndef __INCLUDE_FS_RASTERIZE_GRID_GLSL__
#define __INCLUDE_FS_RASTERIZE_GRID_GLSL__

#include "scalable_kinect_fusion/common_tracking.glsl"

vec3 Indexto3D(int Index, int Resolution)
{
    int z = Index / (Resolution * Resolution);
    Index -= (z * Resolution * Resolution);
    int y = Index / Resolution;
    int x = Index % Resolution;
    return vec3(x, y, z);
}

// -----------------------------------------------------------------------------
// Shader storage buffers
// -----------------------------------------------------------------------------

layout(std430, binding = 0) buffer AtomicCounterBuffer
{
    uint g_Counters[];
};

// -----------------------------------------------------------------------------
// Uniform buffers
// -----------------------------------------------------------------------------

layout(row_major, std140, binding = 2) uniform UBOTransform
{
    ivec3 g_Offset;
    int g_Resolution;
    float g_CubeSize;
    float g_ParentSize;
};

// -----------------------------------------------------------------------------
// Images
// -----------------------------------------------------------------------------

layout(binding = 0, MAP_TEXTURE_FORMAT) readonly uniform image2D cs_Vertex;

// -----------------------------------------------------------------------------
// Input
// -----------------------------------------------------------------------------

layout(location = 0) in flat uint in_Index;
layout(location = 1) in vec3 in_AABBMin;
layout(location = 2) in vec3 in_AABBMax;

// -----------------------------------------------------------------------------
// Output
// -----------------------------------------------------------------------------

layout(location = 0) out vec4 out_Color;

// -----------------------------------------------------------------------------
// Functions
// -----------------------------------------------------------------------------

bool InBox()
{
    vec3 Vertex = imageLoad(cs_Vertex, ivec2(gl_FragCoord.xy)).xyz;
    Vertex = (g_PoseMatrix * vec4(Vertex, 1.0f)).xyz; 

    return 
        Vertex.x > in_AABBMin.x && Vertex.x < in_AABBMax.x &&
        Vertex.y > in_AABBMin.y && Vertex.y < in_AABBMax.y &&
        Vertex.z > in_AABBMin.z && Vertex.z < in_AABBMax.z;
}

// -----------------------------------------------------------------------------
// Fragment shader
// -----------------------------------------------------------------------------

void main()
{
    bool InBox = InBox();

    if (InBox)
    {
        atomicAdd(g_Counters[in_Index], 1);
    }
    out_Color = InBox ? vec4(1.0f) : vec4(0.5f);
}

#endif // __INCLUDE_FS_RASTERIZE_GRID_GLSL__
