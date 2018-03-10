
#ifndef __INCLUDE_VS_RAYCAST_GLSL__
#define __INCLUDE_VS_RAYCAST_GLSL__

#include "common_global.glsl"

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
	out_WSRayDirection = (g_ViewToWorld * g_ScreenToView * vec4(in_VertexPosition, 1.0f)).xyz - g_ViewPosition.xyz;

    gl_Position = vec4(in_VertexPosition * 2.0f - 1.0f, 1.0f);
}

#endif // __INCLUDE_VS_RAYCAST_GLSL__