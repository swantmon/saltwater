
#ifndef __INCLUDE_FS_SMAA_GLSL__
#define __INCLUDE_FS_SMAA_GLSL__

#include "common_global.glsl"

// -----------------------------------------------------------------------------
// Defines / Settings
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Output to fragment
// -----------------------------------------------------------------------------
layout(location = 0) out vec4 out_Output;


void main(void)
{
    out_Output = vec4(0.5f, 0.5f, 0.5f, 1.0f);
}

#endif // __INCLUDE_FS_SMAA_GLSL__