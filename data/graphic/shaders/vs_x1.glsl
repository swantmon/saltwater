#ifndef __INCLUDE_VS_X1_GLSL__
#define __INCLUDE_VS_X1_GLSL__

layout(row_major, std140, binding = 1) uniform UB0
{
    mat4 m_ViewMatrix;
    mat4 m_ProjectionMatrix;
};

layout(row_major, std140, binding = 2) uniform UB1
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
layout(location = 0) in vec3 in_Position;
layout(location = 1) in vec3 in_Normal;
layout(location = 2) in vec3 in_Tangent;
layout(location = 3) in vec3 in_Bitangent;
layout(location = 4) in vec2 in_UV;

// -----------------------------------------------------------------------------
// Output to next stage
// -----------------------------------------------------------------------------
layout(location = 0) out vec3 out_Position;
layout(location = 1) out vec3 out_Normal;
layout(location = 2) out vec2 out_UV;
layout(location = 3) out mat3 out_WSNormalMatrix;

// -----------------------------------------------------------------------------
// Main
// -----------------------------------------------------------------------------
void main(void)
{
    vec4 WSPosition  = m_ModelMatrix * vec4(in_Position , 1.0f);
    vec4 WSNormal    = m_ModelMatrix * vec4(in_Normal   , 0.0f);
    vec4 WSTangent   = m_ModelMatrix * vec4(in_Tangent  , 0.0f);
    vec4 WSBitangent = m_ModelMatrix * vec4(in_Bitangent, 0.0f);
    
    mat3 WSNormalMatrix = mat3(WSTangent.xyz, WSBitangent.xyz, WSNormal.xyz);
    
    out_Position       = (m_ViewMatrix * WSPosition).xyz;
    out_Normal         = WSNormal.xyz;
    out_UV             = in_UV;
    out_WSNormalMatrix = WSNormalMatrix;
    
    gl_Position      = m_ViewMatrix * WSPosition;
}

#endif // __INCLUDE_VS_X1_GLSL__