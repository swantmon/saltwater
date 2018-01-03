
#ifndef __INCLUDE_FS_LIQUID_SHADING_GLSL__
#define __INCLUDE_FS_LIQUID_SHADING_GLSL__

#include "common.glsl"

// -----------------------------------------------------------------------------
// Input from engine
// -----------------------------------------------------------------------------
layout(row_major, std140, binding = 2) uniform UB2
{
    mat4 ps_InvertedProjectionMatrix;
    mat4 ps_InvertedViewMatrix;
    vec4 ps_LightDirection;
    vec4 g_ViewDirection;
    vec2 ps_InvertedScreensize;
};

layout(binding = 0) uniform samplerCube ps_Environment;
layout(binding = 1) uniform sampler2D   ps_Background;
layout(binding = 2) uniform sampler2D   ps_Depth;
layout(binding = 3) uniform sampler2D   ps_Thickness;

// -----------------------------------------------------------------------------
// Output to fragment
// -----------------------------------------------------------------------------
layout(location = 0) out vec4 PSColor;

// -----------------------------------------------------------------------------
// Input from vertex shader
// -----------------------------------------------------------------------------
in vec2 PSTexCoord;

// -----------------------------------------------------------------------------
// Function
// -----------------------------------------------------------------------------
void main(void)
{
    float Depth       = texture(ps_Depth, PSTexCoord).x;
    float Thickness   = texture(ps_Thickness, PSTexCoord).x;
    
    Thickness = Thickness * 1.0f / 80.0f;
    
    if (Depth == 1.0f)
    {
        discard;
        return;
    }
    
    // -----------------------------------------------------------------------------
    // Calculate view-space position from depth
    // -----------------------------------------------------------------------------
    vec3 VSPosition = GetViewSpacePositionFromDepth(Depth, PSTexCoord, ps_InvertedProjectionMatrix);
    
    // -----------------------------------------------------------------------------
    // Calculate neighbor depth values
    // -----------------------------------------------------------------------------
    vec2 TexCoordXP = PSTexCoord + vec2(  ps_InvertedScreensize.x, 0.0f);
    vec2 TexCoordXN = PSTexCoord + vec2( -ps_InvertedScreensize.x, 0.0f);
    
    vec2 TexCoordYP = PSTexCoord + vec2( 0.0f,  ps_InvertedScreensize.y);
    vec2 TexCoordYN = PSTexCoord + vec2( 0.0f, -ps_InvertedScreensize.y);
    
    float DepthXP = texture(ps_Depth, TexCoordXP).x;
    float DepthXN = texture(ps_Depth, TexCoordXN).x;
    
    float DepthYP = texture(ps_Depth, TexCoordYP).x;
    float DepthYN = texture(ps_Depth, TexCoordYN).x;
    
    // -----------------------------------------------------------------------------
    // Calculate differences
    // -----------------------------------------------------------------------------
    vec3 ddx  = GetViewSpacePositionFromDepth(DepthXP, TexCoordXP, ps_InvertedProjectionMatrix) - VSPosition;
    vec3 ddx2 = VSPosition - GetViewSpacePositionFromDepth(DepthXN, TexCoordXN, ps_InvertedProjectionMatrix);
    
    if (abs(ddx.z) > abs(ddx2.z))
    {
        ddx = ddx2;
    }
    
    vec3 ddy  = GetViewSpacePositionFromDepth(DepthYP, TexCoordYP, ps_InvertedProjectionMatrix) - VSPosition;
    vec3 ddy2 = VSPosition - GetViewSpacePositionFromDepth(DepthYN, TexCoordYN, ps_InvertedProjectionMatrix);
    
    if (abs(ddy2.z) < abs(ddy.z))
    {
        ddy = ddy2;
    }
    
    // -----------------------------------------------------------------------------
    // Calculate normal
    // -----------------------------------------------------------------------------
    vec3 VSNormal = cross(ddx, ddy);
    
    VSNormal = normalize(VSNormal);
    
    vec3 WSNormal = (ps_InvertedViewMatrix * vec4(VSNormal, 0.0f)).xyz;
    
    WSNormal = normalize(WSNormal);
    
    // -----------------------------------------------------------------------------
    // Calculate diffuse color
    // -----------------------------------------------------------------------------
    vec3  WSLightDirection  = -normalize(ps_LightDirection.xyz);
    vec3  WSViewDirection   = normalize(g_ViewDirection.xyz);
    vec3  WSHalfVector      = normalize(WSViewDirection + WSLightDirection);
    
    // -----------------------------------------------------------------------------
    // Calculate cubemap reflection
    // -----------------------------------------------------------------------------
    vec3  Reflection = reflect(-WSViewDirection, WSNormal);
    
    vec3  CubeReflection = texture(ps_Environment, Reflection).rgb;
    
    // -----------------------------------------------------------------------------
    // Fresnel approximmation
    // -----------------------------------------------------------------------------
    const float FresnelReflection = 0.05f;
    float Fresnel = FresnelReflection + (1.0f - FresnelReflection) * pow(1.0f - dot(WSHalfVector, WSViewDirection), 5.0f);
    
    // -----------------------------------------------------------------------------
    // Beer's law + adding environment: Color due to Absorption
    // Almost no red, leave a bit of green, leave most of blue
    // -----------------------------------------------------------------------------
    vec3 Beer = vec3(exp(-5.0 * Thickness),
                     exp(-3.0 * Thickness),
                     exp(-0.2 * Thickness));
    
    // -----------------------------------------------------------------------------
    // Intensities
    // -----------------------------------------------------------------------------
    float DiffuseIntensity  = max(0.0f, dot(WSNormal.xyz, WSLightDirection) * 0.5f + 0.5f);
    float SpecularIntensity = pow(max(0.0f, dot(WSNormal, WSHalfVector)), 6.0f);
    
    // -----------------------------------------------------------------------------
    // Schlick's approximation for liquid specular
    // -----------------------------------------------------------------------------
    float Schlick = clamp(SpecularIntensity + (1.0f - SpecularIntensity) * pow(1.0f - abs(dot(WSNormal, WSLightDirection)), 8.0f), 0.0f, 1.0f);
    
    // -----------------------------------------------------------------------------
    // Background Image Refracted in 2D
    // -----------------------------------------------------------------------------
    vec3 Background = texture(ps_Background, PSTexCoord + VSNormal.xy * Thickness).rgb;
    
    // -----------------------------------------------------------------------------
    // Shading
    // -----------------------------------------------------------------------------
    float Transparency = 1.0f - Thickness;
    
    vec3 DiffuseColor  = mix(DiffuseIntensity * Beer, Background, Transparency);
    vec3 SpecularColor = Schlick * Fresnel * CubeReflection;
    
    PSColor = vec4(clamp(DiffuseColor + SpecularColor, 0.0f, 1.0f), 1.0f);
}

#endif // __INCLUDE_FS_LIQUID_SHADING_GLSL__