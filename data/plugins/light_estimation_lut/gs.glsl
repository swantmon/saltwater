
#ifndef __INCLUDE_GS_SPHERICAL_ENV_CUBEMAP_GENERATION_GLSL__
#define __INCLUDE_GS_SPHERICAL_ENV_CUBEMAP_GENERATION_GLSL__

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
layout(std140, binding = 0) uniform UB2
{
    mat4 m_CubeProjectionMatrix;
    mat4 m_CubeViewMatrix[6];
};

layout(std140, binding = 1) uniform UB3
{
    mat4 m_ModelMatrix;
};

// -----------------------------------------------------------------------------
// Input from previous stage
// -----------------------------------------------------------------------------
layout(location = 0) in vec3 in_Normal[];

// -----------------------------------------------------------------------------
// Output to next stage
// -----------------------------------------------------------------------------
layout(location = 0) out vec3 out_Normal;

// -----------------------------------------------------------------------------
// Main
// -----------------------------------------------------------------------------
void main() 
{
    for( int FaceIndex = 0; FaceIndex < 6; ++FaceIndex )
    {
        for( int IndexOfVertex = 0; IndexOfVertex < 3; IndexOfVertex++ )
        {
            gl_Layer = FaceIndex;
            
            out_Normal  = -in_Normal[IndexOfVertex];
            gl_Position = m_CubeProjectionMatrix * m_CubeViewMatrix[FaceIndex] * m_ModelMatrix * gl_in[IndexOfVertex].gl_Position;

            EmitVertex();
        }

        EndPrimitive();
    }
}

#endif // __INCLUDE_GS_SPHERICAL_ENV_CUBEMAP_GENERATION_GLSL__