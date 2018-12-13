
#ifndef __INCLUDE_CS_PLANE_EQUATION_GLSL__
#define __INCLUDE_CS_PLANE_EQUATION_GLSL__

#include "slam/scalable_kinect_fusion/plane_detection/common_plane_detection.glsl"

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

layout (local_size_x = TILE_SIZE2D, local_size_y = TILE_SIZE2D, local_size_z = 1) in;
void main()
{
    const int x = int(gl_GlobalInvocationID.x);
    const int y = int(gl_GlobalInvocationID.y);
    const int PlaneIndex = int(gl_WorkGroupID.z);

    ivec2 ImageSize = imageSize(cs_VertexMap);
    if (x >= ImageSize.x || y >= ImageSize.y)
    {
        return;
    }

    vec3 Plane = normalize(g_Planes[PlaneIndex].xyz);
    vec3 Normal = mat3(g_InvPoseMatrix) * normalize(imageLoad(cs_NormalMap, ivec2(x, y)).xyz);
    vec3 Vertex = (g_InvPoseMatrix * vec4(imageLoad(cs_VertexMap, ivec2(x, y)).xyz, 1.0f)).xyz;
    
    if (abs(dot(Plane, Normal)) > 0.1f)
    {
        float D = dot(Plane, Vertex);
    
        int Bin = PlaneDistanceToBin(D, g_AzimuthBinCount);
    
        imageAtomicAdd(cs_Histogram, ivec2(Bin, PlaneIndex), 1);
    }
}

#endif // __INCLUDE_CS_PLANE_EQUATION_GLSL__