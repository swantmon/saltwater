
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

layout(location = 0) out vec3 out_WSRayDirection;

out gl_PerVertex
{
    vec4 gl_Position;
};

void main()
{
    vec4 CSPosition = VertexData[gl_VertexID];
    vec4 VSPosition = g_ScreenToView * CSPosition;
    VSPosition /= VSPosition.w;
    vec4 WSPosition = g_ViewToWorld * VSPosition;

    out_WSRayDirection = normalize(WSPosition.xyz - g_ViewPosition.xyz);

    gl_Position = CSPosition;
}

#endif // __INCLUDE_VS_KINECT_RAYCAST_GLSL__