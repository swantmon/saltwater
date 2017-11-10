
#ifndef __INCLUDE_FS_RAYCAST_LEVEL1_GLSL__
#define __INCLUDE_FS_RAYCAST_LEVEL1_GLSL__

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

vec3 GetRootVolumeOffset(vec3 GlobalPosition)
{
    return GlobalPosition - mod(GlobalPosition, VOLUME_SIZE);
}

int GetRootVolumeBufferIndex(vec3 GlobalPosition)
{
    vec3 BufferPosition = GlobalPosition / VOLUME_SIZE + g_VolumeTextureWidth / 2.0f;
    uint VolumeIndex = OffsetToIndex(BufferPosition, g_VolumeTextureWidth);
    return g_RootVolumePositionBuffer[VolumeIndex];
}

int GetRootGridItemIndex(vec3 PositionInVolume, int VolumeBufferOffset)
{    
    ivec3 ItemOffset = ivec3(PositionInVolume / (VOLUME_SIZE / 16.0f));
    ivec3 VolumeOffset = ItemOffset % 16;
    
    int BufferOffset = VolumeOffset.z * 16 * 16 + VolumeOffset.y * 16 + VolumeOffset.x;
    
    return VolumeBufferOffset * 16 * 16 * 16 + BufferOffset;
}

int GetLevel1GridItemIndex(vec3 PositionInVolume, int VolumeBufferOffset)
{    
    ivec3 ItemOffset = ivec3(PositionInVolume / (VOLUME_SIZE / (16.0f * 8.0f)));
    ivec3 VolumeOffset = ItemOffset % 8;
    
    int BufferOffset = VolumeOffset.z * 8 * 8 + VolumeOffset.y * 8 + VolumeOffset.x;
    
    return VolumeBufferOffset * 8 * 8 * 8 + BufferOffset;
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

        // Index of the first element of the current rootgrid in the rootgrid pool
        int VolumeBufferOffset = GetRootVolumeBufferIndex(CurrentPosition);

        if (VolumeBufferOffset != -1)
        {
            // Global offset of the rootvolume
            vec3 VolumeOffset = g_RootVolumePool[VolumeBufferOffset].m_Offset * VOLUME_SIZE;

            // Index to rootgrid pool of the current root grid item
            int RootGridItemBufferOffset = GetRootGridItemIndex(CurrentPosition - VolumeOffset, VolumeBufferOffset);
            
            if (RootGridItemBufferOffset != -1)
            {
                // Pool index of whole level 1 grid
                int Level1VolumeBufferOffset = g_RootGridPool[RootGridItemBufferOffset].m_PoolIndex;

                if (Level1BufferOffset != -1)
                {
                    out_GBuffer0 = vec4(1.0f, 0.0f, 0.0f, 1.0f);
                    out_GBuffer1 = vec4(1.0f, 0.0f, 0.0f, 1.0f);
                    out_GBuffer2 = vec4(0.0f, 0.0f, 0.0f, 1.0f);
                
                    return;
                }
            }
        }
    }
    
    //vec4 CSPosition = g_WorldToScreen * vec4(CurrentPosition, 1.0f);
    //gl_FragDepth = (CSPosition.z / CSPosition.w);

    discard;
}

#endif // __INCLUDE_FS_RAYCAST_LEVEL1_GLSL__