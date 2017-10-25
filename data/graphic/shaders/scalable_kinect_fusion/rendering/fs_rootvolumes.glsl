
#ifndef __INCLUDE_FS_ROOTVOLUMES_GLSL__
#define __INCLUDE_FS_ROOTVOLUMES_GLSL__

#include "common_global.glsl"
#include "scalable_kinect_fusion/common_scalable.glsl"
#include "common_gbuffer.glsl"

// -----------------------------------------------------------------------------
// Input from engine
// -----------------------------------------------------------------------------

layout(std430, binding = 0) buffer RootVolumeBuffer
{
    uint g_RootVolumeBuffer[];
};

layout(row_major, std140, binding = 1) uniform ScalableRaycastConstantBuffer
{
    vec3 m_AABBMin;
    vec3 m_AABBMax;
    int m_VolumeTextureWidth;
};

// -----------------------------------------------------------------------------
// Input from previous shader stage
// -----------------------------------------------------------------------------

layout(location = 0) in vec3 in_WSRayDirection;

layout(location = 0) out vec4 out_GBuffer0;
layout(location = 1) out vec4 out_GBuffer1;
layout(location = 2) out vec4 out_GBuffer2;

// -----------------------------------------------------------------------------
// Helper functions
// -----------------------------------------------------------------------------

float GetStartLength(vec3 Start, vec3 Direction)
{
    float xmin = ((Direction.x > 0.0f ? 0.0f : VOLUME_SIZE) - Start.x) / Direction.x;
    float ymin = ((Direction.y > 0.0f ? 0.0f : VOLUME_SIZE) - Start.y) / Direction.y;
    float zmin = ((Direction.z > 0.0f ? 0.0f : VOLUME_SIZE) - Start.z) / Direction.z;

    return max(max(xmin, ymin), zmin);
}

float GetEndLength(vec3 Start, vec3 Direction)
{
    float xmax = ((Direction.x > 0.0f ? VOLUME_SIZE : 0.0f) - Start.x) / Direction.x;
    float ymax = ((Direction.y > 0.0f ? VOLUME_SIZE : 0.0f) - Start.y) / Direction.y;
    float zmax = ((Direction.z > 0.0f ? VOLUME_SIZE : 0.0f) - Start.z) / Direction.z;

    return min(min(xmax, ymax), zmax);
}

void main()
{
    vec3 RayDirection = normalize(in_WSRayDirection);

    RayDirection.x = RayDirection.x == 0.0f ? 1e-15f : RayDirection.x;
    RayDirection.y = RayDirection.y == 0.0f ? 1e-15f : RayDirection.y;
    RayDirection.z = RayDirection.z == 0.0f ? 1e-15f : RayDirection.z;

    float StartLength = GetStartLength(g_ViewPosition.xyz, RayDirection);
    float EndLength = GetEndLength(g_ViewPosition.xyz, RayDirection);

    out_GBuffer0 = vec4(1.0f, 0.0f, 0.0f, 1.0f);
    out_GBuffer1 = vec4(1.0f, 0.0f, 0.0f, 1.0f);
    out_GBuffer2 = vec4(1.0f, 0.0f, 0.0f, 1.0f);
}

#endif // __INCLUDE_FS_ROOTVOLUMES_GLSL__