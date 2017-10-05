
#ifndef __INCLUDE_FS_RASTERIZE_ROOTVOLUME_GLSL__
#define __INCLUDE_FS_RASTERIZE_ROOTVOLUME_GLSL__

#include "scalable_kinect_fusion/common_tracking.glsl"

// -----------------------------------------------------------------------------
// Shader storage buffers
// -----------------------------------------------------------------------------

struct SInstanceData
{
    ivec3 m_Offset;
    int m_Index;
};

layout(std430, binding = 0) buffer AtomicBuffer
{
    uint g_Counters[];
};

layout(std430, binding = 1) buffer InstanceBuffer
{
    SInstanceData g_InstanceData[];
};

// -----------------------------------------------------------------------------
// Images
// -----------------------------------------------------------------------------

layout(binding = 0, MAP_TEXTURE_FORMAT) readonly uniform image2D cs_Vertex;

// -----------------------------------------------------------------------------
// Input
// -----------------------------------------------------------------------------

layout(location = 0) in flat int in_Index;

// -----------------------------------------------------------------------------
// Output
// -----------------------------------------------------------------------------

layout(location = 0) out vec4 out_Color;

// -----------------------------------------------------------------------------
// Functions
// -----------------------------------------------------------------------------

bool InBox()
{
    vec3 AABBPosition = g_InstanceData[in_Index].m_Offset * VOLUME_SIZE;
    
    vec3 AABBMin = AABBPosition;
    vec3 AABBMax = AABBPosition + VOLUME_SIZE;

    vec3 Vertex = imageLoad(cs_Vertex, ivec2(gl_FragCoord)).xyz;
    Vertex = (g_PoseMatrix * vec4(Vertex, 1.0f)).xyz; 

    return 
        Vertex.x != 0.0f &&
        Vertex.x > AABBMin.x && Vertex.x < AABBMax.x &&
        Vertex.y > AABBMin.y && Vertex.y < AABBMax.y &&
        Vertex.z > AABBMin.z && Vertex.z < AABBMax.z;
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
}

#endif // __INCLUDE_FS_RASTERIZE_ROOTVOLUME_GLSL__
