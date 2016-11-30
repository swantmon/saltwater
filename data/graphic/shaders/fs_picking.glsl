
#ifndef __INCLUDE_FS_PICKING_GLSL_
#define __INCLUDE_FS_PICKING_GLSL_

// -----------------------------------------------------------------------------
// Input from engine
// -----------------------------------------------------------------------------
layout(std140, binding = 0) uniform UB0
{
    vec4 ps_ColorID;
};

// -----------------------------------------------------------------------------
// Output to fragment
// -----------------------------------------------------------------------------
layout(location = 0) out vec4 out_Output;


void main(void)
{
    out_Output = ps_ColorID;
}

#endif // __INCLUDE_FS_PICKING_GLSL_