
#ifndef __INCLUDE_FS_CAUSTIC_GATHERING_GLSL__
#define __INCLUDE_FS_CAUSTIC_GATHERING_GLSL__

// -----------------------------------------------------------------------------
// Input from engine
// -----------------------------------------------------------------------------
layout(binding = 0) uniform sampler2D ps_InputTexture;

// -----------------------------------------------------------------------------
// Input to fragment from VS
// -----------------------------------------------------------------------------
layout(location = 2) in vec2 in_UV;

// -----------------------------------------------------------------------------
// Output to fragment
// -----------------------------------------------------------------------------
layout(location = 0) out vec4 out_Output;

// -----------------------------------------------------------------------------
// Function
// -----------------------------------------------------------------------------
void main(void)
{
    out_Output = vec4(texture(ps_InputTexture, in_UV).rgb, 1.0f);
}

#endif // __INCLUDE_FS_CAUSTIC_GATHERING_GLSL__