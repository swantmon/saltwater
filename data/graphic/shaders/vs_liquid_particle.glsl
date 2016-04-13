
#ifndef __INCLUDE_VS_LIQUID_PARTICLE_GLSL__
#define __INCLUDE_VS_LIQUID_PARTICLE_GLSL__

#include "vs_global.glsl"

layout(row_major, std140, binding = 1) uniform UConstantBufferVS1 /* UPerFrameConstantBuffer */
{
    mat4 m_LiquidMatrix;
};

layout(location=0) in vec2 VertexPosition;
layout(location=1) in vec3 VertexOffset;

out vec3 PSWSPosition;
out vec2 PSTexCoord;

out gl_PerVertex
{
    vec4 gl_Position;
};

void main(void)
{
    vec4 WSPosition = m_LiquidMatrix * vec4(VertexPosition.xy, 0.0f, 1.0f);
    
    WSPosition.xyz += VertexOffset;
    
    PSWSPosition = WSPosition.xyz;
    PSTexCoord   = vec2(VertexPosition.xy);
    
    gl_Position = vs_ViewProjectionMatrix * WSPosition;
}

#endif // __INCLUDE_VS_LIQUID_PARTICLE_GLSL__