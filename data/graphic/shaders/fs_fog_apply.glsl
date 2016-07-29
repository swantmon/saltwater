
#ifndef __INCLUDE_FS_FOG_APPLY_GLSL__
#define __INCLUDE_FS_FOG_APPLY_GLSL__

#include "common_global.glsl"

// -----------------------------------------------------------------------------
// Input to fragment from VS
// -----------------------------------------------------------------------------
layout(binding = 0) uniform sampler2D ps_LightAccumulation;
layout(binding = 1) uniform sampler2D ps_DepthTexture;
layout(binding = 2) uniform sampler3D ps_VolumeScattering;

layout(location = 2) in vec2 in_UV;

// -----------------------------------------------------------------------------
// Output to fragment
// -----------------------------------------------------------------------------
layout(location = 0) out vec4 out_Output;

void main(void)
{
    vec4 Light = texture(ps_LightAccumulation, in_UV);

    float Depth = texture(ps_DepthTexture, in_UV).r;
    
    float zNear = ps_CameraParameterNear;
    float zFar = ps_CameraParameterFar;
    float LinDepth2 = 2*zFar*zNear / (zFar + zNear - (zFar - zNear)*(2*Depth -1));

    vec4 Scattering = texture(ps_VolumeScattering, vec3(in_UV, LinDepth2 / 128.0f));
    
    vec3 InScattering = Scattering.rgb;
    
    float Transmittance = Scattering.a;

    out_Output = vec4(Light.rgb * Transmittance.xxx + InScattering, Light.a);
}

#endif // __INCLUDE_FS_FOG_APPLY_GLSL__