
#ifndef __INCLUDE_VS_SMAA_GLSL__
#define __INCLUDE_VS_SMAA_GLSL__

const vec2 Vertices[3] = vec2[3]
(
	vec2(-1.0, -1.0),
	vec2( 3.0, -1.0),
	vec2(-1.0,  3.0)
);

out gl_PerVertex
{
    vec4 gl_Position;
};

void main(void)
{
    gl_Position = vec4(Vertices[gl_VertexID], 0.0, 1.0);
}

#endif // __INCLUDE_VS_SMAA_GLSL__