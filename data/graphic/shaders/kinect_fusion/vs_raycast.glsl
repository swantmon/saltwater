
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

vec4 VertexData[4] =
{
    vec4( 1.0,  1.0, 0.0, 1.0),
    vec4(-1.0,  1.0, 0.0, 1.0),
    vec4(-1.0, -1.0, 0.0, 1.0),
    vec4( 1.0, -1.0, 0.0, 1.0)
};

layout(location = 0) in vec3 in_VertexPosition;

layout(location = 0) out vec3 out_WSRayDirection;

out gl_PerVertex
{
    vec4 gl_Position;
};

void main()
{
    out_WSRayDirection = normalize(in_VertexPosition.xyz - g_ViewPosition.xyz);

    gl_Position = g_WorldToScreen * vec4(in_VertexPosition, 1.0f);
}

#endif // __INCLUDE_VS_KINECT_RAYCAST_GLSL__