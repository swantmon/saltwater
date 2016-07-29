
#ifndef __INCLUDE_FS_DOF_APPLY_GLSL__
#define __INCLUDE_FS_DOF_APPLY_GLSL__

#include "common_global.glsl"

// -----------------------------------------------------------------------------
// Input from engine
// -----------------------------------------------------------------------------
layout(binding = 0) uniform sampler2D ps_Color;
layout(binding = 1) uniform sampler2D ps_Depth;
layout(binding = 2) uniform sampler2D ps_NearBlur;
layout(binding = 3) uniform sampler2D ps_DownBlur;

layout(binding = 1) uniform UB1
{
    vec4 ps_DofLerpScale;
    vec4 ps_DofLerpBias;
    vec3 ps_DofEqFar;
};

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
float ConvertToLinearDepth(float _HyperbolicDepth, float _Near, float _Far)
{
    return (2.0f * _Near) / (_Far + _Near - _HyperbolicDepth * (_Far - _Near));
}

// -----------------------------------------------------------------------------

vec3 GetSmallBlurSample(vec2 _TexCoords)
{
    const float Weight = 4.0f / 17.0f;
    
    vec3 Blurred;
    
    Blurred = vec3(0.0f);
    
    Blurred += Weight * texture( ps_Color, _TexCoords + vec2(+0.5f, -1.5f) * ps_InvertedScreensizeAndScreensize.xy).rgb;
    Blurred += Weight * texture( ps_Color, _TexCoords + vec2(-1.5f, -0.5f) * ps_InvertedScreensizeAndScreensize.xy).rgb;
    Blurred += Weight * texture( ps_Color, _TexCoords + vec2(-0.5f, +1.5f) * ps_InvertedScreensizeAndScreensize.xy).rgb;
    Blurred += Weight * texture( ps_Color, _TexCoords + vec2(+1.5f, +0.5f) * ps_InvertedScreensizeAndScreensize.xy).rgb;
    
    return Blurred;
}

// -----------------------------------------------------------------------------

vec4 InterpolateDof(vec3 _SmallBlur, vec3 _MediumBlur, vec3 _LargeBlur, float _CircleOfConfusion)
{
    vec4 Weights;
    vec3 Color;
    
    float Alpha;
    
    Weights = clamp(_CircleOfConfusion * ps_DofLerpScale + ps_DofLerpBias, 0.0f, 1.0f);
    
    Weights.yz = min(Weights.yz, vec2(1.0f) - Weights.xy);
    
    Color = Weights.z * _SmallBlur + Weights.y * _MediumBlur + Weights.x * _LargeBlur;
    
    Alpha = dot(Weights.zyx, vec3( 16.0f / 17.0f, 1.0f, 1.0f ));

    return vec4(Color, Alpha);
}

// -----------------------------------------------------------------------------

vec4 ApplyDepthOfField(vec2 _TexCoords)
{
    // -----------------------------------------------------------------------------
    // Initialize variables
    // -----------------------------------------------------------------------------
    vec3  Small;
    vec4  Medium;
    vec3  Large;
    float Depth;
    float NearCoc;
    float FarCoc;
    float Coc;
    
    // -----------------------------------------------------------------------------
    // Get small blur
    // -----------------------------------------------------------------------------
    Small = GetSmallBlurSample( _TexCoords );
    
    // -----------------------------------------------------------------------------
    // Get medium blur
    // -----------------------------------------------------------------------------
    Medium = texture(ps_NearBlur, _TexCoords);
    
    // -----------------------------------------------------------------------------
    // Get large blur
    // -----------------------------------------------------------------------------
    Large = texture(ps_DownBlur, _TexCoords).rgb;
    
    // -----------------------------------------------------------------------------
    // Get near coc
    // -----------------------------------------------------------------------------
    NearCoc = Medium.a;
    
    // -----------------------------------------------------------------------------
    // Get depth and check value
    // -----------------------------------------------------------------------------
    Depth = ConvertToLinearDepth(texture(ps_Depth, _TexCoords).r, 0.01f, 4096.0f);
    
    if (Depth > 1.0f)
    {
        Coc = NearCoc;
    }
    else
    {
        // -----------------------------------------------------------------------------
        // m_DofFar.x and m_DofFar.y specify the linear ramp to convert to depth for
        // the distant out-of-focus region.
        // m_DofFar.z is the ratio of the far to the near blur radius.
        // -----------------------------------------------------------------------------
        FarCoc = clamp(ps_DofEqFar.x * Depth + ps_DofEqFar.y, 0.0f, 1.0f);
        
        Coc = max(NearCoc, FarCoc * ps_DofEqFar.z);
    }
    
    // -----------------------------------------------------------------------------
    // Calculate dof image
    // -----------------------------------------------------------------------------
    return InterpolateDof(Small, Medium.rgb, Large, Coc);
}

// -----------------------------------------------------------------------------

void main(void)
{
    // -----------------------------------------------------------------------------
    // Return final color with pre multiplied alpha blending to result
    // -----------------------------------------------------------------------------
    out_Output = ApplyDepthOfField(in_UV);
}

#endif // __INCLUDE_FS_DOF_APPLY_GLSL__