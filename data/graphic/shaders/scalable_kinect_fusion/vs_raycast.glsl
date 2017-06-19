
#ifndef __INCLUDE_VS_KINECT_RAYCAST_GLSL__
#define __INCLUDE_VS_KINECT_RAYCAST_GLSL__

#include "common_global.glsl"

// -----------------------------------------------------------------------------
// Constants
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Input from engine
// -----------------------------------------------------------------------------

layout(row_major, std140, binding = 1) uniform PerDrawCallData
{
    vec4 g_Offset;
    vec4 g_Color;
};

// -----------------------------------------------------------------------------
// Functions
// -----------------------------------------------------------------------------

layout(location = 0) in vec3 in_VertexPosition;

layout(location = 0) out vec3 out_WSRayDirection;

out gl_PerVertex
{
    vec4 gl_Position;
};

void main()
{
	const vec3 WSPosition = (in_VertexPosition + g_Offset.xyz) * VOLUME_SIZE;

    out_WSRayDirection = WSPosition - g_ViewPosition.xyz;

    gl_Position = g_WorldToScreen * vec4(WSPosition, 1.0f);
}

#endif // __INCLUDE_VS_KINECT_RAYCAST_GLSL__