
#ifndef __INCLUDE_FS_SHADOW_GLSL__
#define __INCLUDE_FS_SHADOW_GLSL__

// -----------------------------------------------------------------------------
// Input from engine
// -----------------------------------------------------------------------------
layout(binding = 0) uniform sampler2D in_PSTextureDiffuse;

// -----------------------------------------------------------------------------
// Input to fragment from VS
// -----------------------------------------------------------------------------
layout(location = 0) in vec3 in_WSPosition;
layout(location = 1) in vec3 in_WSNormal;
layout(location = 2) in vec2 in_UV;

// -----------------------------------------------------------------------------
// Output to fragment
// -----------------------------------------------------------------------------
layout(location = 0) out vec4 out_Position;
layout(location = 1) out vec4 out_Normal;
layout(location = 2) out vec4 out_Flux;

// -----------------------------------------------------------------------------
// Function
// -----------------------------------------------------------------------------
void SM(void)
{

}

// -----------------------------------------------------------------------------

void RSM(void)
{
    out_Position = vec4(in_WSPosition, 1.0f);
    out_Normal   = vec4(normalize(in_WSNormal), 1.0f);
    out_Flux     = vec4(texture(in_PSTextureDiffuse, in_UV).rgb, 1.0f);
}

#endif // __INCLUDE_FS_SHADOW_GLSL__