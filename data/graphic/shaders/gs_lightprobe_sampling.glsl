
#ifndef __INCLUDE_GS_LIGHTPROBE_SAMPLING_GLSL__
#define __INCLUDE_GS_LIGHTPROBE_SAMPLING_GLSL__

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
layout(triangle_strip, max_vertices = 18) out;

// -----------------------------------------------------------------------------
// Input from engine
// -----------------------------------------------------------------------------
layout(row_major, std140, binding = 0) uniform UConstantBufferGS0 /* UPerFrameConstantBufferGS */
{
    mat4 m_CubeProjectionMatrix;
    mat4 m_CubeViewMatrix[6];
};

// -----------------------------------------------------------------------------
// Input from previous stage
// -----------------------------------------------------------------------------
layout(location = 0) in vec3 in_Normal[];
layout(location = 1) in vec2 in_TexCoord[];

// -----------------------------------------------------------------------------
// Output to next stage
// -----------------------------------------------------------------------------
layout(location = 0) out vec3 out_Normal;
layout(location = 1) out vec2 out_TexCoord;

// -----------------------------------------------------------------------------
// Main
// -----------------------------------------------------------------------------
void main() 
{
    for( int FaceIndex = 0; FaceIndex < 6; ++FaceIndex )
    {
        gl_Layer = FaceIndex;

        for( int IndexOfVertex = 0; IndexOfVertex < 3; IndexOfVertex++ )
        {
            out_Normal   = -in_Normal[IndexOfVertex];
            out_TexCoord = in_TexCoord[IndexOfVertex];
            gl_Position  = m_CubeProjectionMatrix * m_CubeViewMatrix[FaceIndex] * gl_in[IndexOfVertex].gl_Position;

            EmitVertex();
        }

        EndPrimitive();
    }
}

#endif // __INCLUDE_GS_LIGHTPROBE_SAMPLING_GLSL__