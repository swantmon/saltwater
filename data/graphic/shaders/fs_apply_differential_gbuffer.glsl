#ifndef __INCLUDE_FS_APPLY_DIFFERENTIAL_GBUFFER_GLSL_
#define __INCLUDE_FS_APPLY_DIFFERENTIAL_GBUFFER_GLSL_

#include "common_gbuffer.glsl"
#include "common_global.glsl"

// -----------------------------------------------------------------------------
// Input from engine
// -----------------------------------------------------------------------------
layout(binding = 0) uniform sampler2D ps_Texture0;
layout(binding = 1) uniform sampler2D ps_Texture1;

// -----------------------------------------------------------------------------
// Input from previous stage
// -----------------------------------------------------------------------------
layout(location = 2) in vec2 in_UV;

// -----------------------------------------------------------------------------
// Output to fragment
// -----------------------------------------------------------------------------
layout(location = 0) out vec4 out_Output;

// -----------------------------------------------------------------------------

void main()
{
    // -----------------------------------------------------------------------------
    // Color
    // -----------------------------------------------------------------------------
    vec4 Color1 = textureLod(ps_Texture0, in_UV, 0);
    vec4 Color2 = textureLod(ps_Texture1, in_UV, 0);

    out_Output = Color2 - Color1;
}

#endif // __INCLUDE_FS_APPLY_DIFFERENTIAL_GBUFFER_GLSL_