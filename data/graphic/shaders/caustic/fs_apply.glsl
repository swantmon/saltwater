#ifndef __INCLUDE_FS_APPLY_GLSL__
#define __INCLUDE_FS_APPLY_GLSL__

#include "common.glsl"
#include "common_global.glsl"

// -----------------------------------------------------------------------------
// Input from engine
// -----------------------------------------------------------------------------
layout(std140, binding = 1) uniform UB1
{
    mat4 ps_LightProjectionMatrix;
    mat4 ps_LightViewMatrix;
    uint ps_ExposureHistoryIndex;
};

layout(std430, binding = 0) buffer UExposureHistoryBuffer
{
    float ps_ExposureHistory[8];
};

layout(binding = 0) uniform sampler2D ps_Depth;
layout(binding = 1) uniform sampler2D ps_PhotonTexture;

// -----------------------------------------------------------------------------
// Input to fragment prev. stage
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
    // -----------------------------------------------------------------------------
    // Get data
    // -----------------------------------------------------------------------------
    float VSDepth  = texture(ps_Depth   , in_UV).r;

    // -----------------------------------------------------------------------------
    // VS position
    // -----------------------------------------------------------------------------
    vec3 VSPosition = GetViewSpacePositionFromDepth(VSDepth, in_UV, g_ScreenToView);
    
    // -----------------------------------------------------------------------------
    // WS position
    // -----------------------------------------------------------------------------
    vec3 WSPosition = (g_ViewToWorld * vec4(VSPosition, 1.0f)).xyz;

    // -----------------------------------------------------------------------------
    // Exposure data
    // -----------------------------------------------------------------------------
    float AverageExposure = ps_ExposureHistory[ps_ExposureHistoryIndex];

    // -----------------------------------------------------------------------------
    // Set worls space coord into light projection by multiply with light
    // view and projection matrix;
    // -----------------------------------------------------------------------------
    vec4 LSPosition;
    
    LSPosition = ps_LightProjectionMatrix * ps_LightViewMatrix * vec4(WSPosition, 1.0f);
    
    // -----------------------------------------------------------------------------
    // Divide xyz by w to get the position in light view's clip space.
    // -----------------------------------------------------------------------------
    if (LSPosition.z < 0.0f) return;

    LSPosition.xyz /= LSPosition.w;
    
    // -----------------------------------------------------------------------------
    // Get uv texcoords for this position
    // -----------------------------------------------------------------------------
    vec2 LSUV;

    LSUV.x = LSPosition.x * 0.5f + 0.5f;
    LSUV.y = LSPosition.y * 0.5f + 0.5f;

    vec3 Illumination = texture(ps_PhotonTexture, LSUV).xyz;

    out_Output = vec4(Illumination * AverageExposure, 0.0f);
}

#endif // __INCLUDE_FS_APPLY_GLSL__