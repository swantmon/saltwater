
#ifndef __INCLUDE_VS_POINT_CLOUD_GLSL__
#define __INCLUDE_VS_POINT_CLOUD_GLSL__

#include "common_global.glsl"

layout(std140, binding = 1) uniform PerDrawCallData
{
    mat4 g_WorldMatrix;
    vec4 g_Color;
};

layout (binding = 0, MAP_TEXTURE_FORMAT) uniform image2D cs_VertexMap;

out gl_PerVertex
{
    vec4 gl_Position;
    float gl_PointSize;
};

// -----------------------------------------------------------------------------
// Functions
// -----------------------------------------------------------------------------

void main()
{
    mat3 Rot2 = mat3(
        1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f,
        0.0f, -1.0f, 0.0f
    );

    ivec2 UV;
    UV.x = gl_VertexID % DEPTH_IMAGE_WIDTH;
    UV.y = gl_VertexID / DEPTH_IMAGE_WIDTH;
    vec4 WSPosition = imageLoad(cs_VertexMap, UV);
    WSPosition.w = 1.0f;
    WSPosition = g_WorldMatrix * vec4(Rot2 * WSPosition.xyz, 1.0f);
    gl_Position = g_WorldToScreen * WSPosition;
    gl_PointSize = 3.0f;
}

#endif // __INCLUDE_VS_POINT_CLOUD_GLSL__