
#ifndef __INCLUDE_FS_LUT_ENV_CUBEMAP_GENERATION_GLSL__
#define __INCLUDE_FS_LUT_ENV_CUBEMAP_GENERATION_GLSL__

#include "common.glsl"

// -----------------------------------------------------------------------------
// Input from engine
// -----------------------------------------------------------------------------
layout(binding = 0) uniform sampler2D in_InputTexture;
layout(binding = 1) uniform samplerCube in_LookUpTexture;

// -----------------------------------------------------------------------------
// Input to fragment from VS
// -----------------------------------------------------------------------------
layout(location = 0) in vec3 in_Normal;
layout(location = 1) in vec2 in_UV;

// -----------------------------------------------------------------------------
// Output to fragment
// -----------------------------------------------------------------------------
layout(location = 0) out vec4 out_Output;

// -----------------------------------------------------------------------------
// Main
// -----------------------------------------------------------------------------
void main(void)
{
	vec4 LookUp = textureLod(in_LookUpTexture, in_Normal, 0.0f);

    vec4 FinalColor = texture(in_InputTexture, LookUp.xy);
        
    out_Output = vec4(LookUp.xyz, 1.0f);
}

#endif // __INCLUDE_FS_LUT_ENV_CUBEMAP_GENERATION_GLSL__