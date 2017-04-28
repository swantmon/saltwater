#ifndef __INCLUDE_VS_X1_GLSL__
#define __INCLUDE_VS_X1_GLSL__

layout(row_major, std140, binding = 0) uniform UB0
{
    mat4 m_ViewMatrix;
    mat4 m_ProjectionMatrix;
};

layout(row_major, std140, binding = 1) uniform UB1
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
layout(location = 0) in vec3 VertexPosition;
layout(location = 1) in vec2 VertexTexCoord;

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
    out_UV = vec2(VertexTexCoord.x, 1.0f - VertexTexCoord.y);

    vec4 WSPosition = inverse(m_ViewMatrix) * m_ModelMatrix * vec4(VertexPosition.xyz, 1.0f);

    gl_Position = WSPosition;
}

#endif // __INCLUDE_VS_X1_GLSL__