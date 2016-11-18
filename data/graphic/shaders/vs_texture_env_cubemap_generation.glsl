
#ifndef __INCLUDE_VS_TEXTURE_ENV_CUBEMAP_GENERATION_GLSL__
#define __INCLUDE_VS_TEXTURE_ENV_CUBEMAP_GENERATION_GLSL__

layout(row_major, std140, binding = 0) uniform UB0
{
    mat4 m_ModelMatrix;
};

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
layout(location = 0) in vec2 VertexPosition;

// -----------------------------------------------------------------------------
// Output to next stage
// -----------------------------------------------------------------------------
layout(location = 0) out vec3 out_Normal;
layout(location = 1) out vec2 out_UV;

// -----------------------------------------------------------------------------
// Main
// -----------------------------------------------------------------------------
void main(void)
{
    vec4 Position = vec4(VertexPosition.xy, 0.0f, 1.0f);

    out_UV = vec2(Position.x, 1.0f - Position.y);

    vec4 WSPosition = m_ModelMatrix * vec4(Position.xyz, 1.0f);
    
    out_Normal = normalize(WSPosition.xyz);
    
    gl_Position = WSPosition;
}

#endif // __INCLUDE_VS_TEXTURE_ENV_CUBEMAP_GENERATION_GLSL__