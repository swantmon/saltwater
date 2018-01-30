
#ifndef __INCLUDE_CS_PLANE_EQUATION_GLSL__
#define __INCLUDE_CS_PLANE_EQUATION_GLSL__

#include "scalable_kinect_fusion/plane_detection/common_plane_detection.glsl"

layout(std430, binding = 0) buffer PlaneCountBuffer
{
    int g_PlaneIndex;
    int g_MaxPlaneCount;
    ivec2 Padding;
};

layout(std430, binding = 1) buffer PlaneBuffer
{
    vec4 g_Planes[];
};

// -----------------------------------------------------------------------------
// Input from engine
// -----------------------------------------------------------------------------

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

    vec4 Plane = g_Planes[g_PlaneIndex];
    vec3 Normal = imageLoad(cs_NormalMap, ivec2(x, y)).xyz;

    imageStore(cs_NormalMap, ivec2(x, y), vec4(0.5));
}

#endif // __INCLUDE_CS_PLANE_EQUATION_GLSL__