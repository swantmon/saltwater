
#ifndef __INCLUDE_VS_OUTLINE_GLSL__
#define __INCLUDE_VS_OUTLINE_GLSL__

layout(location = 0) in vec2 in_VertexPosition;

layout(location = 0) out vec2 out_TexCoord;
layout(location = 1) out vec2 out_TexCoordsFlipped;

out gl_PerVertex
{
    vec4 gl_Position;
};

// -----------------------------------------------------------------------------
// Functions
// -----------------------------------------------------------------------------

void main()
{
    out_TexCoord = out_TexCoordsFlipped = in_VertexPosition * 0.5f + 0.5f;
    out_TexCoordsFlipped.y = 1.0f - out_TexCoord.y;
    gl_Position = vec4(in_VertexPosition, 0.0f, 1.0f);
}

#endif // __INCLUDE_VS_OUTLINE_GLSL__