
#ifndef __INCLUDE_FS_LIQUID_DEPTH_GLSL__
#define __INCLUDE_FS_LIQUID_DEPTH_GLSL__

#include "functions.glsl"

// -----------------------------------------------------------------------------
// Input from engine
// -----------------------------------------------------------------------------
layout(row_major, std140, binding = 0) uniform ULiquidSettings
{
    mat4  ps_ViewMatrix;
    mat4  ps_ProjectionMatrix;
    vec4  ps_LightDirection;
    vec4  ps_Color;
    float ps_SphereRadius;
};

// -----------------------------------------------------------------------------
// Output to fragment
// -----------------------------------------------------------------------------
layout(location = 0) out float out_Depth;

// -----------------------------------------------------------------------------
// Input from vertex shader
// -----------------------------------------------------------------------------
in vec3 PSWSPosition;
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
    
    Normal.z = sqrt(1.0f - Radius);
    
    // -----------------------------------------------------------------------------
    // Calculate depth
    // -----------------------------------------------------------------------------
    vec4 VSPosition = ps_ViewMatrix * vec4(PSWSPosition, 1.0f);
    vec4 CSPosition = ps_ProjectionMatrix * vec4(VSPosition.xyz + Normal * ps_SphereRadius, 1.0f);
    
    out_Depth = CSPosition.z / CSPosition.w;
}

#endif // __INCLUDE_FS_LIQUID_DEPTH_GLSL__