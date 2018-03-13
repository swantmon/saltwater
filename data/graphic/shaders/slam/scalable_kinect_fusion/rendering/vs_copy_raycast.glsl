
#ifndef __INCLUDE_VS_RAYCAST_GLSL__
#define __INCLUDE_VS_RAYCAST_GLSL__

#include "common_global.glsl"

// -----------------------------------------------------------------------------
// Functions
// -----------------------------------------------------------------------------

layout(location = 0) in vec3 in_VertexPosition;

out gl_PerVertex
{
    vec4 gl_Position;
};

void main()
{
    gl_Position = vec4(in_VertexPosition.xy * 2.0f - 1.0f, 0.0f, 1.0f);
}

#endif // __INCLUDE_VS_RAYCAST_GLSL__