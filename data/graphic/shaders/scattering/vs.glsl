
#ifndef __INCLUDE_VS_GLSL__
#define __INCLUDE_VS_GLSL__

layout(location = 2) out vec2 out_UV;

out gl_PerVertex
{
    vec4 gl_Position;
};

void main()
{
    vec2 Vertices[] =
    {
        vec2(-1.0f, -1.0f),
        vec2( 3.0f, -1.0f),
        vec2(-1.0f,  3.0f),
    };

    out_UV      = Vertices[gl_VertexID] / vec2(2.0f) + 0.5f;
	gl_Position = vec4(Vertices[gl_VertexID], 0.0f, 1.0f);
}

#endif // __INCLUDE_VS_GLSL__