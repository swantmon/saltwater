
#ifndef __INCLUDE_FS_RASTERIZE_GRID_GLSL__
#define __INCLUDE_FS_RASTERIZE_GRID_GLSL__

#include "scalable_kinect_fusion/common_tracking.glsl"

uvec3 Indexto3D(uint Index, uint Resolution)
{
    uint z = Index / (Resolution * Resolution);
    Index -= (z * Resolution * Resolution);
    uint y = Index / Resolution;
    uint x = Index % Resolution;
    return uvec3(x, y, z);
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
    uint g_Resolution;
    float g_CubeSize;
    ivec3 g_Offset;
};

// -----------------------------------------------------------------------------
// Images
// -----------------------------------------------------------------------------

layout(binding = 0, MAP_TEXTURE_FORMAT) readonly uniform image2D cs_Vertex;

// -----------------------------------------------------------------------------
// Input
// -----------------------------------------------------------------------------

layout(location = 0) in flat uint in_Index;

// -----------------------------------------------------------------------------
// Output
// -----------------------------------------------------------------------------

layout(location = 0) out vec4 out_Color;

// -----------------------------------------------------------------------------
// Functions
// -----------------------------------------------------------------------------

bool InBox()
{
    vec3 AABBPosition = g_Offset * float(VOLUME_SIZE);
    AABBPosition += Indexto3D(in_Index, g_Resolution) * g_CubeSize;
    
    vec3 AABBMin = AABBPosition;
    vec3 AABBMax = AABBPosition + g_CubeSize;

    vec3 Vertex = imageLoad(cs_Vertex, ivec2(gl_FragCoord.xy)).xyz;
    Vertex = (g_PoseMatrix * vec4(Vertex, 1.0f)).xyz; 

    return 
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
    out_Color = InBox ? vec4(1.0f) : vec4(0.5f);
}

#endif // __INCLUDE_FS_RASTERIZE_GRID_GLSL__
