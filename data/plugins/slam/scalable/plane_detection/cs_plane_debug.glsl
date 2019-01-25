
#ifndef __INCLUDE_CS_PLANE_EQUATION_GLSL__
#define __INCLUDE_CS_PLANE_EQUATION_GLSL__

#include "../../plugins/slam/scalable/plane_detection/common_plane_detection.glsl"

layout(std430, binding = 0) buffer PlaneCountBuffer
{
    uint g_IndirectX;
    uint g_IndirectY;
    uint g_IndirectZ;
    uint g_MaxPlaneCount;
};

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

    imageStore(cs_VertexMap, ivec2(x, y), vec4(0.0f));
}

#endif // __INCLUDE_CS_PLANE_EQUATION_GLSL__