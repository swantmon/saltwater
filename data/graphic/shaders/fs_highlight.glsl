
#ifndef __INCLUDE_FS_SELECTION_GLSL_
#define __INCLUDE_FS_SELECTION_GLSL_

// -----------------------------------------------------------------------------
// Input from engine
// -----------------------------------------------------------------------------
layout(std140, binding = 0) uniform UB0
{
    vec4 ps_Color;
};

// -----------------------------------------------------------------------------
// Output to fragment
// -----------------------------------------------------------------------------
layout(location = 0) out vec4 out_Output;


void main(void)
{
    out_Output = ps_Color;
}

#endif // __INCLUDE_FS_SELECTION_GLSL_