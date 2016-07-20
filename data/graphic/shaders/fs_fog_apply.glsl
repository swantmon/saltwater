
#ifndef __INCLUDE_FS_FOG_APPLY_GLSL__
#define __INCLUDE_FS_FOG_APPLY_GLSL__

#include "fs_global.glsl"

// -----------------------------------------------------------------------------
// Input to fragment from VS
// -----------------------------------------------------------------------------
layout(location = 2) in vec2 in_UV;

// -----------------------------------------------------------------------------
// Output to fragment
// -----------------------------------------------------------------------------
layout(location = 0) out vec4 out_Output;

void main(void)
{
    out_Output = vec4(1, 0, 0, 0);
}

#endif // __INCLUDE_FS_FOG_APPLY_GLSL__