
#ifndef __INCLUDE_GS_RASTERIZATION_ROOTGRID_GLSL__
#define __INCLUDE_GS_RASTERIZATION_ROOTGRID_GLSL__

// -----------------------------------------------------------------------------
// Buffer
// -----------------------------------------------------------------------------

layout(row_major, std140, binding = 0) uniform PerVolumeData
{
    ivec3 g_Offset;
    int g_Resolution;
};

// -----------------------------------------------------------------------------
// In- and outputs
// -----------------------------------------------------------------------------

in 
{gl_PerVertex
    vec4 gl_Position;
} gl_in[];

out gl_PerVertex
{
    vec4 gl_Position;
};

layout(location = 0) in vec3 in_WSPosition[];

// -----------------------------------------------------------------------------
// Geometry shader
// -----------------------------------------------------------------------------

layout(points) in;
layout(points, max_vertices = 1) out;
void main()
{
    gl_Position = gl_in[0].gl_Position + vec4(-0.1, 0.0, 0.0, 0.0);
    EmitVertex();

    EndPrimitive();
}

#endif // __INCLUDE_GS_RASTERIZATION_ROOTGRID_GLSL__