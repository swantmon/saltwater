
#ifndef __INCLUDE_FS_SSAO_APPLY_GLSL__
#define __INCLUDE_FS_SSAO_APPLY_GLSL__

// -----------------------------------------------------------------------------
// Input from engine
// -----------------------------------------------------------------------------
layout(binding = 0) uniform sampler2D ps_GBuffer0;
layout(binding = 1) uniform sampler2D ps_GBuffer1;
layout(binding = 2) uniform sampler2D ps_GBuffer2;
layout(binding = 3) uniform sampler2D ps_SSAO;

// -----------------------------------------------------------------------------
// Input to fragment from VS
// -----------------------------------------------------------------------------
layout(location = 2) in vec2 in_TexCoord;

// -----------------------------------------------------------------------------
// Output to fragment
// -----------------------------------------------------------------------------
layout(location = 0) out vec4 out_GBuffer0;
layout(location = 1) out vec4 out_GBuffer1;
layout(location = 2) out vec4 out_GBuffer2;

// -----------------------------------------------------------------------------
// Function
// -----------------------------------------------------------------------------
void main(void)
{
    vec4 GBuffer0 = texture(ps_GBuffer0, in_TexCoord);
    vec4 GBuffer1 = texture(ps_GBuffer1, in_TexCoord);
    vec4 GBuffer2 = texture(ps_GBuffer2, in_TexCoord);
    
    // -----------------------------------------------------------------------------
    // Change deferred buffer
    // -----------------------------------------------------------------------------
    vec3 SSAO = texture(ps_SSAO, in_TexCoord).rgb;
    
    GBuffer2.w *= SSAO.x;
    
    out_GBuffer0 = GBuffer0;
    out_GBuffer1 = GBuffer1;
    out_GBuffer2 = GBuffer2;
}

#endif // __INCLUDE_FS_SSAO_APPLY_GLSL__