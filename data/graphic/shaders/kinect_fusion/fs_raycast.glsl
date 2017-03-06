
#ifndef __INCLUDE_FS_KINECT_RAYCAST_GLSL__
#define __INCLUDE_FS_KINECT_RAYCAST_GLSL__

#include "common_global.glsl"
#include "common_raycast.glsl"

// -----------------------------------------------------------------------------
// Constants
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Input from engine
// -----------------------------------------------------------------------------

layout(row_major, std140, binding = 1) uniform PerDrawCallData
{
    vec4 g_LightPosition;
    vec4 g_Color;
};

layout (binding = 0) uniform isampler3D fs_Volume;

layout(location = 0) in vec3 in_WSRayDirection;

layout(location = 0) out vec4 out_Color;

void main()
{
    vec3 RayDirection = normalize(in_WSRayDirection);

    RayDirection.x = RayDirection.x == 0.0f ? 1e-15f : RayDirection.x;
    RayDirection.y = RayDirection.y == 0.0f ? 1e-15f : RayDirection.y;
    RayDirection.z = RayDirection.z == 0.0f ? 1e-15f : RayDirection.z;

    vec3 WSPosition = GetPosition(g_ViewPosition.xyz, RayDirection, fs_Volume);
    
    if (WSPosition.x != 0.0f)
    {
        vec3 WSNormal = GetNormal(WSPosition, fs_Volume);

        vec3 WSLightDirection = normalize(WSPosition - g_LightPosition.xyz);
        vec3 WSHalf = normalize(WSLightDirection + in_WSRayDirection);

        float DiffuseIntensity = max(0.0f, dot(WSNormal, WSLightDirection)) * 0.6f;
        float SpecularIntensity = max(0.0f, pow(max(0.0f, dot(WSNormal, WSHalf)), 127.0f));
        float LightIntensity = DiffuseIntensity + SpecularIntensity + 0.2f;

        out_Color = vec4(g_Color.xyz * LightIntensity, 1.0f);
    }
    else
    {
        discard;
    }
}

#endif // __INCLUDE_FS_KINECT_RAYCAST_GLSL__