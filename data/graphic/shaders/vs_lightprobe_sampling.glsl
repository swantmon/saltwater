
#ifndef __INCLUDE_VS_LIGHTPROBE_SAMPLING_GLSL__
#define __INCLUDE_VS_LIGHTPROBE_SAMPLING_GLSL__

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
layout(location = 1) in vec3 VertexNormal;
layout(location = 2) in vec2 VertexTexCoord;

// -----------------------------------------------------------------------------
// Output to next stage
// -----------------------------------------------------------------------------
layout(location = 1) out vec3 out_Normal;
layout(location = 2) out vec2 out_UV;

// -----------------------------------------------------------------------------
// Main
// -----------------------------------------------------------------------------
void main(void)
{
    vec4 WSPosition = vec4(VertexPosition.xyz, 1.0f);
    vec3 WSNormal   = VertexNormal;
    
    out_Normal = normalize(WSNormal);
    out_UV     = VertexTexCoord;
    
    gl_Position = WSPosition;
}

#endif // __INCLUDE_VS_LIGHTPROBE_SAMPLING_GLSL__