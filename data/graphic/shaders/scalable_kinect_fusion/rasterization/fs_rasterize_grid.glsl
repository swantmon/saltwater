
#ifndef __INCLUDE_FS_RASTERIZE_GRID_GLSL__
#define __INCLUDE_FS_RASTERIZE_GRID_GLSL__

#include "scalable_kinect_fusion/common_tracking.glsl"
#include "scalable_kinect_fusion/common_indirect.glsl"

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

layout(std430, binding = 3) buffer VolumeQueue
{
    uint g_VolumeID[];
};

layout(std430, binding = 4) buffer AtomicBuffer
{
    uint g_Counters[];
};

layout(std430, binding = 5) buffer IndirectBuffer
{
    SIndirectBuffers g_Indirect;
};

// -----------------------------------------------------------------------------
// Images
// -----------------------------------------------------------------------------

layout(binding = 0, MAP_TEXTURE_FORMAT) readonly uniform image2D cs_Vertex;

// -----------------------------------------------------------------------------
// Input
// -----------------------------------------------------------------------------

layout(location = 0) in flat uint in_Index;
layout(location = 1) in flat vec3 in_AABBMin;
layout(location = 2) in flat vec3 in_AABBMax;

// -----------------------------------------------------------------------------
// Output
// -----------------------------------------------------------------------------

layout(location = 0) out vec4 out_Color;

// -----------------------------------------------------------------------------
// Functions
// -----------------------------------------------------------------------------

bool InBox()
{
    vec3 Vertex = imageLoad(cs_Vertex, ivec2(gl_FragCoord)).xyz;
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
        uint Count = atomicAdd(g_Counters[in_Index], 1);
        if (Count == 1)
        {
            uint Index = atomicAdd(g_Indirect.m_Indexed.m_InstanceCount, 1);
            g_VolumeID[Index] = in_Index;
        }
    }
}

#endif // __INCLUDE_FS_RASTERIZE_GRID_GLSL__
