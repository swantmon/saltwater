
#ifndef __INCLUDE_FS_COLOR_SPHERE_ALPHA_GLSL__
#define __INCLUDE_FS_COLOR_SPHERE_ALPHA_GLSL__

// -----------------------------------------------------------------------------
// Input from engine
// -----------------------------------------------------------------------------
layout(binding = 0) uniform UB0
{
    vec4 ps_Color;
};

// -----------------------------------------------------------------------------
// Input from previous stage
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
    vec2 Normal;
    
    Normal.xy = in_UV * vec2(2.0f, -2.0f);
    
    float Radius = dot(Normal.xy, Normal.xy);
    
    if (Radius > 1.0f)
    {
        discard;
    }

    out_Output = vec4(ps_Color);
}

#endif // __INCLUDE_FS_COLOR_SPHERE_ALPHA_GLSL__