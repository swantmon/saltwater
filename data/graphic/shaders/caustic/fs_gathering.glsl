
#ifndef __INCLUDE_FS_GATHERING_GLSL__
#define __INCLUDE_FS_GATHERING_GLSL__

// -----------------------------------------------------------------------------
// Output to fragment
// -----------------------------------------------------------------------------
layout(location = 0) out vec4 out_Output;

// -----------------------------------------------------------------------------
// Function
// -----------------------------------------------------------------------------
void main(void)
{
    out_Output = vec4(1.0f);
}

#endif // __INCLUDE_FS_GATHERING_GLSL__