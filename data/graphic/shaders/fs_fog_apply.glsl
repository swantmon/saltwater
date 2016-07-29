
#ifndef __INCLUDE_FS_FOG_APPLY_GLSL__
#define __INCLUDE_FS_FOG_APPLY_GLSL__

#include "common.glsl"
#include "common_global.glsl"

// -----------------------------------------------------------------------------
// Input from engine
// -----------------------------------------------------------------------------
layout(row_major, std140, binding = 1) uniform UB1
{
    float cs_FrustumDepthInMeter;
};

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

    float LinearDepth = ConvertToLinearDepth(texture(ps_DepthTexture, in_UV).r, ps_CameraParameterNear, ps_CameraParameterFar);

    float UVz = clamp(LinearDepth * ps_CameraParameterFar / cs_FrustumDepthInMeter, 0.0f, 1.0f);

    vec4 Scattering = texture(ps_VolumeScattering, vec3(in_UV, UVz));
    
    vec3 InScattering = Scattering.rgb;
    
    float Transmittance = Scattering.a;

    out_Output = vec4(Light.rgb * Transmittance.xxx + InScattering, Light.a);
}

#endif // __INCLUDE_FS_FOG_APPLY_GLSL__