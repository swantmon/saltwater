
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
// Input from engine
// -----------------------------------------------------------------------------
layout(std140, binding = 0) uniform UB0
{
    mat4 m_ModelMatrix;
};

// -----------------------------------------------------------------------------
// Input from buffer
// -----------------------------------------------------------------------------
layout(location = 0) in vec3 VertexPosition;

// -----------------------------------------------------------------------------
// Output to next stage
// -----------------------------------------------------------------------------
layout(location = 0) out vec3 out_Normal;

// -----------------------------------------------------------------------------
// Main
// -----------------------------------------------------------------------------
void main(void)
{
    vec4 WSPosition = vec4(VertexPosition.xyz, 1.0f);
    
    out_Normal = -normalize(WSPosition.xyz);
    
    gl_Position = m_ModelMatrix * WSPosition;
}

#endif // __INCLUDE_VS_SPHERICAL_ENV_CUBEMAP_GENERATION_GLSL__