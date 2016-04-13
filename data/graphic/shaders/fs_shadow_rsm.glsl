
#ifndef __INCLUDE_FS_SHADOW_RSM_GLSL__
#define __INCLUDE_FS_SHADOW_RSM_GLSL__

// -----------------------------------------------------------------------------
// Input from engine
// -----------------------------------------------------------------------------
uniform sampler2D PSTextureDiffuse;

// -----------------------------------------------------------------------------
// Input to fragment from VS
// -----------------------------------------------------------------------------
layout(location = 0) in vec3 PSPosition;
layout(location = 1) in vec3 PSNormal;
layout(location = 2) in vec2 PSTexCoord;

// -----------------------------------------------------------------------------
// Output to fragment
// -----------------------------------------------------------------------------
layout(location = 0) out vec4 PSOutputPosition;
layout(location = 1) out vec4 PSOutputNormal;
layout(location = 2) out vec4 PSOutputFlux;

// -----------------------------------------------------------------------------
// Function
// -----------------------------------------------------------------------------
void main(void)
{
    PSOutputNormal   = vec4(normalize(PSNormal), 1.0f);
    PSOutputFlux     = vec4(texture(PSTextureDiffuse, PSTexCoord).rgb, 1.0f);
    PSOutputPosition = vec4(PSPosition, 1.0f);
}

#endif // __INCLUDE_FS_SHADOW_RSM_GLSL__