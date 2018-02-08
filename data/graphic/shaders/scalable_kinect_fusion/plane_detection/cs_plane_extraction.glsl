
#ifndef __INCLUDE_CS_PLANE_EXTRACTION_GLSL__
#define __INCLUDE_CS_PLANE_EXTRACTION_GLSL__

#include "scalable_kinect_fusion/plane_detection/common_plane_detection.glsl"

layout(std430, binding = 1) buffer PlaneBuffer
{
    vec4 g_Planes[];
};

// -----------------------------------------------------------------------------
// Input from engine
// -----------------------------------------------------------------------------

layout (binding = 0, r32i) uniform iimage2D cs_Histogram;
layout (binding = 1, MAP_TEXTURE_FORMAT) uniform image2D cs_VertexMap;
layout (binding = 2, MAP_TEXTURE_FORMAT) uniform image2D cs_NormalMap;

// -------------------------------------------------------------------------------------
// Functions
// -------------------------------------------------------------------------------------

shared int g_Counts[HISTOGRAM_WIDTH];

layout (local_size_x = HISTOGRAM_WIDTH, local_size_y = 1, local_size_z = 1) in;
void main()
{    
    const int x = int(gl_GlobalInvocationID.x);

    const uint PlaneIndex = gl_WorkGroupID.z;

    const int Count = imageLoad(cs_Histogram, ivec2(gl_GlobalInvocationID.xz)).x;

    g_Counts[gl_LocalInvocationIndex] = Count;

    barrier();

    for (int i = HISTOGRAM_WIDTH / 2; i > 0; i /= 2)
    {
        g_Counts[gl_LocalInvocationIndex] = max(g_Counts[gl_LocalInvocationIndex + i], g_Counts[gl_LocalInvocationIndex]);

        barrier();
    }
    
    if (g_Counts[0] == Count)
    {
        float D = BinToPlaneDistance(int(gl_GlobalInvocationID.x), g_AzimuthBinCount);
        g_Planes[PlaneIndex].w = -D;
    }
}

#endif // __INCLUDE_CS_PLANE_EXTRACTION_GLSL__