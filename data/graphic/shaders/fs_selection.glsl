
#ifndef __INCLUDE_FS_MATERIAL_GLSL_
#define __INCLUDE_FS_MATERIAL_GLSL_

// -----------------------------------------------------------------------------
// Input from engine
// -----------------------------------------------------------------------------
layout(std140, binding = 0) uniform UB0
{
    vec4 ps_Color;
};

// -----------------------------------------------------------------------------
// Input to fragment from VS
// -----------------------------------------------------------------------------
layout(location = 0) in vec3 in_PSVertex;
layout(location = 1) in vec3 in_PSNormal;
layout(location = 2) in vec2 in_PSTexCoord;
layout(location = 3) in mat3 in_PSWSNormalMatrix;

// -----------------------------------------------------------------------------
// Output to fragment
// -----------------------------------------------------------------------------
layout(location = 0) out vec4 out_Output;


void main(void)
{
    out_Output = ps_Color;
}

#endif // __INCLUDE_FS_MATERIAL_GLSL_