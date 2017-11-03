
#ifndef __INCLUDE_FS_ROOTGRIDS_GLSL__
#define __INCLUDE_FS_ROOTGRIDS_GLSL__

#include "common_global.glsl"
#include "scalable_kinect_fusion/common_scalable.glsl"
#include "common_gbuffer.glsl"

// -----------------------------------------------------------------------------
// Input from engine
// -----------------------------------------------------------------------------

layout(row_major, std140, binding = 1) uniform ScalableRaycastConstantBuffer
{
    vec3 g_AABBMin;
    vec3 g_AABBMax;
    int g_VolumeTextureWidth;
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

float GetStartLength(vec3 Start, vec3 Direction, vec3 AABBMin, vec3 AABBMax)
{
    float xmin = ((Direction.x > 0.0f ? AABBMin.x : AABBMax.x) - Start.x) / Direction.x;
    float ymin = ((Direction.y > 0.0f ? AABBMin.y : AABBMax.y) - Start.y) / Direction.y;
    float zmin = ((Direction.z > 0.0f ? AABBMin.z : AABBMax.z) - Start.z) / Direction.z;

    return max(max(xmin, ymin), zmin);
}

float GetEndLength(vec3 Start, vec3 Direction, vec3 AABBMin, vec3 AABBMax)
{
    float xmax = ((Direction.x > 0.0f ? AABBMax.x : AABBMin.x) - Start.x) / Direction.x;
    float ymax = ((Direction.y > 0.0f ? AABBMax.y : AABBMin.y) - Start.y) / Direction.y;
    float zmax = ((Direction.z > 0.0f ? AABBMax.z : AABBMin.z) - Start.z) / Direction.z;

    return min(min(xmax, ymax), zmax);
}

void main()
{
    vec3 CameraPosition = g_ViewPosition.xyz;
    vec3 RayDirection = normalize(in_WSRayDirection);

    RayDirection.x = RayDirection.x == 0.0f ? 1e-15f : RayDirection.x;
    RayDirection.y = RayDirection.y == 0.0f ? 1e-15f : RayDirection.y;
    RayDirection.z = RayDirection.z == 0.0f ? 1e-15f : RayDirection.z;

    const float StartLength = GetStartLength(CameraPosition, RayDirection, g_AABBMin, g_AABBMax);
    const float EndLength = GetEndLength(CameraPosition, RayDirection, g_AABBMin, g_AABBMax);

    float RayLength = StartLength;
    float Step = TRUNCATED_DISTANCE / 1000.0f;

    while (RayLength < EndLength)
    {
        vec3 PreviousPosition = CameraPosition + RayLength * RayDirection;
        RayLength += Step;
        vec3 CurrentPosition = CameraPosition + RayLength * RayDirection;

        vec3 BufferPosition = CurrentPosition / VOLUME_SIZE + g_VolumeTextureWidth / 2.0f;
        uint VolumeIndex = OffsetToIndex(BufferPosition, g_VolumeTextureWidth);
        int Volume = g_RootVolumePositionBuffer[VolumeIndex];

        if (Volume == -1)
        {
            // Compute length where ray leaves volume and directly jump to point

            vec3 AABBMin = g_RootVolumePool[VolumeIndex].m_Offset * VOLUME_SIZE;

            RayLength = GetEndLength(CameraPosition, RayDirection, AABBMin, AABBMin + VOLUME_SIZE);
        }
        else
        {
            
        }
    }

    out_GBuffer0 = vec4(1.0f, 0.0f, 0.0f, 1.0f);
    out_GBuffer1 = vec4(1.0f, 0.0f, 0.0f, 1.0f);
    out_GBuffer2 = vec4(1.0f, 0.0f, 0.0f, 1.0f);
    
    //vec4 CSPosition = g_WorldToScreen * vec4(CurrentPosition, 1.0f);
    //gl_FragDepth = (CSPosition.z / CSPosition.w);

    return;

    discard;
}

#endif // __INCLUDE_FS_ROOTGRIDS_GLSL__