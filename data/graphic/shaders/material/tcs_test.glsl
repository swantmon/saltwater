
#ifndef __INCLUDE_TCS_TEST_GLSL__
#define __INCLUDE_TCS_TEST_GLSL__

#include "common_global.glsl"

// -----------------------------------------------------------------------------
// Built-in variables
// -----------------------------------------------------------------------------
in gl_PerVertex
{
    vec4  gl_Position;
} gl_in[gl_MaxPatchVertices];

out gl_PerVertex
{
    vec4  gl_Position;
} gl_out[];

// -----------------------------------------------------------------------------
// Layout definition
// -----------------------------------------------------------------------------
layout (vertices = 3) out;

// -----------------------------------------------------------------------------
// Input from previous stage
// -----------------------------------------------------------------------------
layout(location = 0) in vec3 in_Position[];
layout(location = 1) in vec3 in_Normal[];
layout(location = 2) in vec2 in_TexCoord[];
layout(location = 3) in mat3 in_WSNormalMatrix[];

// -----------------------------------------------------------------------------
// Output to next stage
// -----------------------------------------------------------------------------
layout(location = 0) out vec3 out_Position[];
layout(location = 1) out vec3 out_Normal[];
layout(location = 2) out vec2 out_TexCoord[];
layout(location = 3) out mat3 out_WSNormalMatrix[];

// -----------------------------------------------------------------------------
// Functions
// -----------------------------------------------------------------------------
float GetTessLevel(float Distance0, float Distance1)
{
    float AvgDistance = (Distance0 + Distance1) / 2.0f;

    if (AvgDistance <= 4.0f)
    {
        return 40.0f;
    }
    else if (AvgDistance <= 10.0f)
    {
        return 12.0f;
    }
    else
    {
        return 1.0f;
    }
}

// -----------------------------------------------------------------------------

void main()
{
    // -----------------------------------------------------------------------------
    // Set the control points of the output patch
    // -----------------------------------------------------------------------------
    out_TexCoord[gl_InvocationID] = in_TexCoord[gl_InvocationID];
    out_Normal  [gl_InvocationID] = in_Normal  [gl_InvocationID];
    out_Position[gl_InvocationID] = in_Position[gl_InvocationID];
    out_WSNormalMatrix[gl_InvocationID] = in_WSNormalMatrix[gl_InvocationID];

    // -----------------------------------------------------------------------------
    // Calculate the distance from the camera to the three control points
    // -----------------------------------------------------------------------------
    float EyeToVertexDistance0 = distance(g_ViewPosition.xyz, out_Position[0]);
    float EyeToVertexDistance1 = distance(g_ViewPosition.xyz, out_Position[1]);
    float EyeToVertexDistance2 = distance(g_ViewPosition.xyz, out_Position[2]);

    // -----------------------------------------------------------------------------
    // Calculate the tessellation levels
    // -----------------------------------------------------------------------------
    gl_TessLevelOuter[0] = GetTessLevel(EyeToVertexDistance1, EyeToVertexDistance2);
    gl_TessLevelOuter[1] = GetTessLevel(EyeToVertexDistance2, EyeToVertexDistance0);
    gl_TessLevelOuter[2] = GetTessLevel(EyeToVertexDistance0, EyeToVertexDistance1);
    gl_TessLevelInner[0] = gl_TessLevelOuter[2];
}

#endif // __INCLUDE_TCS_TEST_GLSL__