
#ifndef __INCLUDE_GS_RASTERIZATION_ROOTGRID_GLSL__
#define __INCLUDE_GS_RASTERIZATION_ROOTGRID_GLSL__

in gl_PerVertex
{
    vec4 gl_Position;
} gl_in[];

out gl_PerVertex
{
    vec4 gl_Position;
};

layout(points) in;
layout(line_strip, max_vertices = 2) out;

void main()
{
    gl_Position = gl_in[0].gl_Position + vec4(-0.1, 0.0, 0.0, 0.0);
    EmitVertex();

    gl_Position = gl_in[0].gl_Position + vec4(0.1, 0.0, 0.0, 0.0);
    EmitVertex();

    EndPrimitive();
}

#endif // __INCLUDE_GS_RASTERIZATION_ROOTGRID_GLSL__