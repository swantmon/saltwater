
#ifndef __INCLUDE_FS_ATMOSPHERE_CUBEMAP_GLSL__
#define __INCLUDE_FS_ATMOSPHERE_CUBEMAP_GLSL__

// -----------------------------------------------------------------------------
// Input from system
// -----------------------------------------------------------------------------
in vec4 gl_FragCoord;

// -----------------------------------------------------------------------------
// Input from engine
// -----------------------------------------------------------------------------
layout(row_major, std140, binding = 8) uniform UB0
{
    vec4 ps_InvertedScreenSize;
    uint ps_ExposureHistoryIndex;
};

layout(std430, binding = 0) readonly buffer BB0
{
    float ps_ExposureHistory[8];
};

layout(binding = 0) uniform samplerCube ps_EnvironmentCube;
layout(binding = 1) uniform sampler2D   ps_Depth;

// -----------------------------------------------------------------------------
// Input
// -----------------------------------------------------------------------------
layout(location = 2) in vec3 in_TexCoord;

// -----------------------------------------------------------------------------
// Output
// -----------------------------------------------------------------------------
layout(location = 0) out vec4 out_Output;

// -----------------------------------------------------------------------------
// Function
// -----------------------------------------------------------------------------
void main(void)
{
    // -----------------------------------------------------------------------------
    // Define tex coords from system input
    // -----------------------------------------------------------------------------
    vec2 TexCoord = vec2(gl_FragCoord.x * ps_InvertedScreenSize.x, gl_FragCoord.y * ps_InvertedScreenSize.y);
    
    // -----------------------------------------------------------------------------
    // Check Depth
    // -----------------------------------------------------------------------------
    float VSDepth = texture(ps_Depth, TexCoord).r;
    
    if (VSDepth < 1.0f)
    {
        discard;
    }
    
    // -----------------------------------------------------------------------------
    // Exposure data
    // -----------------------------------------------------------------------------    
    float AverageExposure = ps_ExposureHistory[ps_ExposureHistoryIndex];
    
    // -----------------------------------------------------------------------------
    // Lighting only if depth is infinity
    // -----------------------------------------------------------------------------
    vec3 Color = texture(ps_EnvironmentCube, in_TexCoord).rgb;
    
//    AverageExposure = 0.0f;
    
    out_Output = vec4(Color * AverageExposure, 1.0f);
}

#endif // __INCLUDE_FS_ATMOSPHERE_CUBEMAP_GLSL__