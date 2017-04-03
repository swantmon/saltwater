
#ifndef __INCLUDE_VS_KINECT_RAYCAST_GLSL__
#define __INCLUDE_VS_KINECT_RAYCAST_GLSL__

#include "common_global.glsl"

// -----------------------------------------------------------------------------
// Constants
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Input from engine
// -----------------------------------------------------------------------------

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
    out_WSRayDirection = in_VertexPosition.xyz - g_ViewPosition.xyz;

    gl_Position = g_WorldToScreen * vec4(in_VertexPosition, 1.0f);
}

#endif // __INCLUDE_VS_KINECT_RAYCAST_GLSL__