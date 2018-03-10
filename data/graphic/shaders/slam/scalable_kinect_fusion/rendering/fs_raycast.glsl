
#ifndef __INCLUDE_FS_RAYCAST_GLSL__
#define __INCLUDE_FS_RAYCAST_GLSL__

#include "common_global.glsl"
#include "slam/scalable_kinect_fusion/common_raycast.glsl"

layout(std140, binding = 1) uniform PerDrawCallData
{
    vec4 g_LightPosition;
    vec4 g_Color;
};

// -----------------------------------------------------------------------------
// Input from previous shader stage
// -----------------------------------------------------------------------------

layout(location = 0) in vec3 in_WSRayDirection;

layout(location = 0) out vec4 out_Intermediate0;
layout(location = 1) out vec4 out_Intermediate1;

// -----------------------------------------------------------------------------
// Helper functions
// -----------------------------------------------------------------------------

void main()
{
    mat3 Rot = mat3(
        1.0f, 0.0f,  0.0f,
        0.0f, 0.0f, -1.0f,
        0.0f, 1.0f,  0.0f
    );

    mat3 Rot2 = mat3(
        1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f,
        0.0f, -1.0f, 0.0f
    );

    vec3 Cameraposition = Rot * g_ViewPosition.xyz;
    vec3 RayDirection = Rot * normalize(in_WSRayDirection);

    RayDirection.x = RayDirection.x == 0.0f ? 1e-15f : RayDirection.x;
    RayDirection.y = RayDirection.y == 0.0f ? 1e-15f : RayDirection.y;
    RayDirection.z = RayDirection.z == 0.0f ? 1e-15f : RayDirection.z;

    vec3 WSPosition, Color;

#ifdef CAPTURE_COLOR

    GetPositionAndColor(Cameraposition, RayDirection, WSPosition, Color);

#else

    WSPosition = Rot2 * GetPosition(Cameraposition, RayDirection);
    Color = g_Color.rgb;

#endif
    
    out_Intermediate0 = vec4(Color, 1.0f);
    out_Intermediate1 = vec4(WSPosition, 1.0f);
}

#endif // __INCLUDE_FS_RAYCAST_GLSL__