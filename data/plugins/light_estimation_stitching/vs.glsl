
#ifndef __INCLUDE_VS_SPHERICAL_ENV_CUBEMAP_GENERATION_GLSL__
#define __INCLUDE_VS_SPHERICAL_ENV_CUBEMAP_GENERATION_GLSL__

// -----------------------------------------------------------------------------
// Built-In variables
// -----------------------------------------------------------------------------
out gl_PerVertex
{
    vec4 gl_Position;
};

// -----------------------------------------------------------------------------
// Input from buffer
// -----------------------------------------------------------------------------
layout(location = 0) in vec3 in_WSPosition;
layout(location = 2) in vec2 in_UV;

// -----------------------------------------------------------------------------
// Output to next stage
// -----------------------------------------------------------------------------
layout(location = 0) out vec2 out_UV;

// -----------------------------------------------------------------------------
// Main
// -----------------------------------------------------------------------------
void main(void)
{
    out_UV = vec2(in_UV.x, 1.0f - in_UV.y);
    
    gl_Position = vec4(in_WSPosition.xyz, 1.0f);
}

#endif // __INCLUDE_VS_SPHERICAL_ENV_CUBEMAP_GENERATION_GLSL__