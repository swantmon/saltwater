
#ifndef __INCLUDE_FS_SSAO_GLSL__
#define __INCLUDE_FS_SSAO_GLSL__

#include "common.glsl"
#include "common_global.glsl"

#define KERNEL_SIZE 16

// -----------------------------------------------------------------------------
// Input from engine
// -----------------------------------------------------------------------------
layout(row_major, std140, binding = 1) uniform UB1
{
    mat4 ps_InverseCameraProjection;
    mat4 ps_CameraProjection;
    mat4 ps_CameraView;
    vec4 ps_NoiseScale;
    vec4 ps_Kernel[KERNEL_SIZE];
};

layout(binding = 0) uniform sampler2D ps_GBuffer0;
layout(binding = 1) uniform sampler2D ps_GBuffer1;
layout(binding = 2) uniform sampler2D ps_Depth;
layout(binding = 3) uniform sampler2D ps_Noise;

// -----------------------------------------------------------------------------
// Input to fragment from VS
// -----------------------------------------------------------------------------
layout(location = 2) in vec2 in_TexCoord;

// -----------------------------------------------------------------------------
// Output to fragment
// -----------------------------------------------------------------------------
layout(location = 0) out vec4 out_Final;

// -----------------------------------------------------------------------------
// Function by Crytek with normal hemisphere
// -----------------------------------------------------------------------------
void main(void)
{
    // -----------------------------------------------------------------------------
    // Read data from texture
    // -----------------------------------------------------------------------------
    vec4  GBuffer0 = texture(ps_GBuffer0, in_TexCoord);
    vec4  GBuffer1 = texture(ps_GBuffer1, in_TexCoord);
    float Depth    = texture(ps_Depth   , in_TexCoord).r;
    
    // -----------------------------------------------------------------------------
    // Linear depth
    // -----------------------------------------------------------------------------
    float LinearDepth = ConvertToLinearDepth(Depth, g_CameraParameterNear, g_CameraParameterFar);
    
    // -----------------------------------------------------------------------------
    // Get world-space normal and convert to view-space normal
    // -----------------------------------------------------------------------------
    vec3 WSNormal = normalize(UnpackNormal(GBuffer0.rg, GBuffer1.a));
    vec4 VSNormal = ps_CameraView * vec4(WSNormal, 0.0f);
    
    // -----------------------------------------------------------------------------
    // Get view-space position from depth
    // -----------------------------------------------------------------------------
    vec3 VSPosition = GetViewSpacePositionFromDepth(Depth, in_TexCoord, g_ScreenToView);
    
    vec2  NoiseScale = ps_NoiseScale.xy;
    float Radius     = ps_NoiseScale.z;
    
    // -----------------------------------------------------------------------------
    // Create random rotation
    // -----------------------------------------------------------------------------
    vec3 RotationSample = texture(ps_Noise, in_TexCoord.xy * NoiseScale).rgb * 2.0f - 1.0f;
    vec3 Tangent        = normalize(RotationSample - VSNormal.xyz * dot(RotationSample, VSNormal.xyz));
    vec3 Bitangent      = cross(Tangent, VSNormal.xyz);
    mat3 KernelBasis    = mat3(Tangent, Bitangent, VSNormal.xyz);
    
    // -----------------------------------------------------------------------------
    // Compute occlusion
    // -----------------------------------------------------------------------------
    vec4  SSOffset          = vec4(1.0f);
    vec3  VSPositionSample  = vec3(1.0f);    
    float DepthSample       = 0.0f;
    float LinearDepthSample = 0.0f;
    float RangeCheck        = 0.0f;
    float Occlusion         = 0.0f;

    for (int IndexOfKernel = 0; IndexOfKernel < KERNEL_SIZE; ++ IndexOfKernel)
    {
        // -----------------------------------------------------------------------------
        // Sample position in view-space
        // -----------------------------------------------------------------------------
        VSPositionSample = KernelBasis * ps_Kernel[IndexOfKernel].xyz;
        
        VSPositionSample = VSPositionSample * Radius + VSPosition;
        
        // -----------------------------------------------------------------------------
        // Offset in screen-space
        // -----------------------------------------------------------------------------
        SSOffset = ps_CameraProjection * vec4(VSPositionSample, 1.0f);
        
        SSOffset.xyz /= SSOffset.w;
        SSOffset.xyz  = SSOffset.xyz * 0.5f + 0.5f;
        
        // -----------------------------------------------------------------------------
        // Sample depth from screen-space texcoord
        // -----------------------------------------------------------------------------
        DepthSample       = texture(ps_Depth, SSOffset.xy).r;
        LinearDepthSample = ConvertToLinearDepth(DepthSample, g_CameraParameterNear, g_CameraParameterFar);
        
        // -----------------------------------------------------------------------------
        // Range check
        // -----------------------------------------------------------------------------
        RangeCheck = smoothstep(0.0f, 1.0f, Radius / (abs(LinearDepth - LinearDepthSample) * g_CameraParameterFar));

        // -----------------------------------------------------------------------------
        // Occlusion check
        // -----------------------------------------------------------------------------
        Occlusion += RangeCheck * step(DepthSample, SSOffset.z);
    }

    Occlusion = 1.0f - (Occlusion / float(KERNEL_SIZE));
    
    out_Final = vec4(pow(Occlusion, 2.0f));
}

#endif // __INCLUDE_FS_SSAO_GLSL__