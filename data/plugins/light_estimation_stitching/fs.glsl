
#ifndef __INCLUDE_FS_TEXTURE_ENV_CUBEMAP_GENERATION_GLSL__
#define __INCLUDE_FS_TEXTURE_ENV_CUBEMAP_GENERATION_GLSL__

#include "common.glsl"
#include "common_global.glsl"

// -----------------------------------------------------------------------------
// Input from engine
// -----------------------------------------------------------------------------
layout(binding = 0) uniform sampler2D in_InputTexture;

// -----------------------------------------------------------------------------
// Input to fragment from VS
// -----------------------------------------------------------------------------
layout(location = 0) in vec2 in_UV;

// -----------------------------------------------------------------------------
// Output to fragment
// -----------------------------------------------------------------------------
layout(location = 0) out vec4 out_Output;

// -----------------------------------------------------------------------------
// Main
// -----------------------------------------------------------------------------
void main(void)
{
	vec2  SSUV   = -1.0f + 2.0f * in_UV;
    float Radius = min(sqrt(dot(SSUV, SSUV)), 1.0f);

    vec4 FinalColor = texture(in_InputTexture, in_UV);

    float Alpha = min((1.0f - Radius), 1.0f) * 1.4f;
    Alpha = clamp(Alpha, 0.0f, 1.0f);
        
    out_Output = vec4(FinalColor.xyz, Alpha);
}

#endif // __INCLUDE_FS_TEXTURE_ENV_CUBEMAP_GENERATION_GLSL__