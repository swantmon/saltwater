
#ifndef __INCLUDE_LIQUID_THICKNESS_GLSL__
#define __INCLUDE_LIQUID_THICKNESS_GLSL__

#include "functions.glsl"

// -----------------------------------------------------------------------------
// Output to fragment
// -----------------------------------------------------------------------------
layout(location = 0) out float out_Thickness;

// -----------------------------------------------------------------------------
// Input from vertex shader
// -----------------------------------------------------------------------------
in vec2 PSTexCoord;

// -----------------------------------------------------------------------------
// Function
// -----------------------------------------------------------------------------
void main(void)
{
    // -----------------------------------------------------------------------------
    // Calculate eye-space sphere normal from texture coordinates
    // -----------------------------------------------------------------------------
    vec3 Normal;
    
    Normal.xy = PSTexCoord * vec2(2.0f, -2.0f);
    
    float Radius = dot(Normal.xy, Normal.xy);
    
    if (Radius > 1.0f)
    {
        discard;
    }
    
    // -----------------------------------------------------------------------------
    // Thickness by gaussian distribution
    // -----------------------------------------------------------------------------
    float Distribution         = Radius;
    float Sigma                = 0.4f;
    float Mean                 = 0.0f;
    float GaussianDistribution = 0.93f * exp(-(Distribution - Mean) * (Distribution - Mean) / (2.0f * Sigma));
    
    out_Thickness = GaussianDistribution;
}

#endif // __INCLUDE_LIQUID_THICKNESS_GLSL__