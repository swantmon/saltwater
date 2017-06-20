
#ifndef __INCLUDE_GS_GLSL__
#define __INCLUDE_GS_GLSL__

// -----------------------------------------------------------------------------
// Built-In variables
// -----------------------------------------------------------------------------
in gl_PerVertex
{
    vec4  gl_Position;
} gl_in[];

out gl_PerVertex
{
    vec4 gl_Position;
};

// -----------------------------------------------------------------------------
// Geometry definition
// -----------------------------------------------------------------------------
layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

// -----------------------------------------------------------------------------
// Input from engine
// -----------------------------------------------------------------------------
layout(row_major, std140, binding = 2) uniform UB0
{
    uint m_Layer;
};

// -----------------------------------------------------------------------------
// Input from previous stage
// -----------------------------------------------------------------------------
layout(location = 2) in vec2 in_UV[];

// -----------------------------------------------------------------------------
// Output to next stage
// -----------------------------------------------------------------------------
layout(location = 2) out vec2 out_UV;
layout(location = 4) out flat uint out_Layer;

// -----------------------------------------------------------------------------
// Main
// -----------------------------------------------------------------------------
void main() 
{
    gl_Layer = int(m_Layer);

    for( int IndexOfVertex = 0; IndexOfVertex < 3; IndexOfVertex++ )
    {
        out_UV    = in_UV[IndexOfVertex];
        out_Layer = m_Layer;
        
        gl_Position = gl_in[IndexOfVertex].gl_Position;

        EmitVertex();
    }

    EndPrimitive();
}

#endif // __INCLUDE_GS_GLSL__