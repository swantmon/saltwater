
#ifndef __INCLUDE_VS_OUTLINE_GLSL__
#define __INCLUDE_VS_OUTLINE_GLSL__

#include "common_global.glsl"

layout(location = 0) in vec3 in_VertexPosition;

out gl_PerVertex
{
    vec4 gl_Position;
};

// -----------------------------------------------------------------------------
// Functions
// -----------------------------------------------------------------------------

void main()
{
    gl_Position = vec4(in_VertexPosition, 1.0f);
}

#endif // __INCLUDE_VS_OUTLINE_GLSL__