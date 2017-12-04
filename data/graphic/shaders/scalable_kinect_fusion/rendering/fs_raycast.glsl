
#ifndef __INCLUDE_FS_RAYCAST_GLSL__
#define __INCLUDE_FS_RAYCAST_GLSL__

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

int GetRootVolumeBufferIndex(vec3 GlobalPosition)
{
    vec3 BufferPosition = GlobalPosition + g_VolumeTextureWidth / 2.0f;
    uint VolumeIndex = OffsetToIndex(BufferPosition, g_VolumeTextureWidth);
    return g_RootVolumePositionBuffer[VolumeIndex];
}

int GetRootGridItemIndex(vec3 PositionInVolume, int VolumeBufferOffset)
{    
    ivec3 ItemOffset = ivec3(floor(PositionInVolume * 16.0f));
    ivec3 VolumeOffset = ItemOffset % 16;
    
    int BufferOffset = VolumeOffset.z * 16 * 16 + VolumeOffset.y * 16 + VolumeOffset.x;
    
    return VolumeBufferOffset * 16 * 16 * 16 + BufferOffset;
}

vec2 GetVoxel(vec3 Position)
{
    // Index of the first element of the current rootgrid in the rootgrid pool
    int VolumeBufferOffset = GetRootVolumeBufferIndex(Position);

    if (VolumeBufferOffset != -1)
    {
        // Global offset of the rootvolume
        vec3 VolumeOffset = g_RootVolumePool[VolumeBufferOffset].m_Offset;

        // Index to rootgrid pool of the current root grid item
        int RootGridItemBufferOffset = GetRootGridItemIndex(Position - VolumeOffset, VolumeBufferOffset);
        
        if (RootGridItemBufferOffset != -1)
        {
            // Pool index of whole level 1 grid                
            int Level1VolumeBufferOffset = g_RootGridPool[RootGridItemBufferOffset].m_PoolIndex;

            if (Level1VolumeBufferOffset != -1)
            {
                // Offset of level 1 volume in rootgrid
                ivec3 Level1VolumeOffset = ivec3(floor(Position * 16.0f * 8.0f));
                Level1VolumeOffset %= 8;

                int Level1BufferInnerOffset = OffsetToIndex(Level1VolumeOffset, 8);
                int Level1BufferIndex = Level1VolumeBufferOffset * 8 * 8 * 8 + Level1BufferInnerOffset;

                int TSDFVolumeBufferOffset = g_Level1GridPool[Level1BufferIndex].m_PoolIndex;

                if (TSDFVolumeBufferOffset != -1)
                {
                    ivec3 TSDFVolumeOffset = ivec3(floor(Position * 16.0f * 8.0f * 8.0f));
                    TSDFVolumeOffset %= 8;

                    int TSDFBufferInnerOffset = OffsetToIndex(TSDFVolumeOffset, 8);
                    int TSDFBufferIndex = TSDFVolumeBufferOffset * 8 * 8 * 8 + TSDFBufferInnerOffset;

                    return unpackSnorm2x16(g_TSDFPool[TSDFBufferIndex]);
                }
            }
        }
    }
    return vec2(0.0f);
}

vec2 GetVoxelFromPosition(vec3 Position)
{
    return GetVoxel(Position / VOLUME_SIZE);
}

float GetInterpolatedTSDF(vec3 Position)
{
    vec3 g = ivec3(floor(Position / VOXEL_SIZE));

    const float vx = (g.x + 0.5f) * VOXEL_SIZE;
    const float vy = (g.y + 0.5f) * VOXEL_SIZE;
    const float vz = (g.z + 0.5f) * VOXEL_SIZE;

    g.x = (Position.x < vx) ? (g.x - 1) : g.x;
    g.y = (Position.y < vy) ? (g.y - 1) : g.y;
    g.z = (Position.z < vz) ? (g.z - 1) : g.z;

    const float a = (Position.x - (g.x + 0.5f) * VOXEL_SIZE) / VOXEL_SIZE;
    const float b = (Position.y - (g.y + 0.5f) * VOXEL_SIZE) / VOXEL_SIZE;
    const float c = (Position.z - (g.z + 0.5f) * VOXEL_SIZE) / VOXEL_SIZE;

    g = (g * VOLUME_SIZE) / 1024.0f;

    const float result =
    GetVoxel(vec3(g.x             , g.y             , g.z             ) * VOLUME_SIZE).x * (1.0f - a) * (1.0f - b) * (1.0f - c) +
    GetVoxel(vec3(g.x             , g.y             , g.z + VOXEL_SIZE) * VOLUME_SIZE).x * (1.0f - a) * (1.0f - b) *         c  +
    GetVoxel(vec3(g.x             , g.y + VOXEL_SIZE, g.z             ) * VOLUME_SIZE).x * (1.0f - a) *         b  * (1.0f - c) +
    GetVoxel(vec3(g.x             , g.y + VOXEL_SIZE, g.z + VOXEL_SIZE) * VOLUME_SIZE).x * (1.0f - a) *         b  *         c  +
    GetVoxel(vec3(g.x + VOXEL_SIZE, g.y             , g.z             ) * VOLUME_SIZE).x *         a  * (1.0f - b) * (1.0f - c) +
    GetVoxel(vec3(g.x + VOXEL_SIZE, g.y             , g.z + VOXEL_SIZE) * VOLUME_SIZE).x *         a  * (1.0f - b) *         c  +
    GetVoxel(vec3(g.x + VOXEL_SIZE, g.y + VOXEL_SIZE, g.z             ) * VOLUME_SIZE).x *         a  *         b  * (1.0f - c) +
    GetVoxel(vec3(g.x + VOXEL_SIZE, g.y + VOXEL_SIZE, g.z + VOXEL_SIZE) * VOLUME_SIZE).x *         a  *         b  *         c ;

    return result;
}

vec3 GetNormal(vec3 Vertex)
{
    vec3 T, Normal;

    T = Vertex;
    T.x += VOXEL_SIZE;
    float Fx1 = GetInterpolatedTSDF(T);
    T = Vertex;
    T.x -= VOXEL_SIZE;
    float Fx2 = GetInterpolatedTSDF(T);

    T = Vertex;
    T.y += VOXEL_SIZE;
    float Fy1 = GetInterpolatedTSDF(T);
    T = Vertex;
    T.y -= VOXEL_SIZE;
    float Fy2 = GetInterpolatedTSDF(T);

    T = Vertex;
    T.z += VOXEL_SIZE;
    float Fz1 = GetInterpolatedTSDF(T);
    T = Vertex;
    T.z -= VOXEL_SIZE;
    float Fz2 = GetInterpolatedTSDF(T);

    Normal.x = Fx2 - Fx1;
    Normal.y = Fy2 - Fy1;
    Normal.z = Fz2 - Fz1;

    return normalize(Normal);
}

void main()
{
    const float TruncatedDistance = TRUNCATED_DISTANCE / 1000.0f;

    vec3 CameraPosition = g_ViewPosition.xyz;
    vec3 RayDirection = normalize(in_WSRayDirection);

    RayDirection.x = RayDirection.x == 0.0f ? 1e-15f : RayDirection.x;
    RayDirection.y = RayDirection.y == 0.0f ? 1e-15f : RayDirection.y;
    RayDirection.z = RayDirection.z == 0.0f ? 1e-15f : RayDirection.z;

    const float StartLength = GetStartLength(CameraPosition, RayDirection, g_AABBMin, g_AABBMax);
    const float EndLength = GetEndLength(CameraPosition, RayDirection, g_AABBMin, g_AABBMax);

    float RayLength = StartLength;
    float Step = TruncatedDistance;

    float CurrentTSDF = GetVoxelFromPosition(CameraPosition + RayLength * RayDirection).x;
    float PreviousTSDF;
    RayLength += Step;

    vec3 Vertex;

    while (RayLength < EndLength)
    {
        vec3 PreviousPosition = CameraPosition + RayLength * RayDirection;
        RayLength += Step;
        vec3 CurrentPosition = CameraPosition + RayLength * RayDirection;

        PreviousTSDF = CurrentTSDF;
        CurrentTSDF = GetVoxelFromPosition(CurrentPosition).x;

        if (CurrentTSDF < 0.0f && PreviousTSDF > 0.0f)
        {
            float Ft = GetInterpolatedTSDF(PreviousPosition);
            float Ftdt = GetInterpolatedTSDF(CurrentPosition);
            float Ts = RayLength - Step * Ft / (Ftdt - Ft);

            Vertex = CameraPosition + RayDirection * Ts;

            break;
        }
        
        Step = CurrentTSDF < 1.0f ? VOXEL_SIZE : TruncatedDistance;

    }

    if (RayLength <= EndLength)
    {
        vec3 Color = vec3(1.0f, 0.0f, 0.0f);

        vec3 WSNormal = GetNormal(Vertex);
                
        WSNormal.x = -WSNormal.x;
        WSNormal.z = -WSNormal.z;
                
        SGBuffer GBuffer;

        PackGBuffer(Color, WSNormal, 0.5f, vec3(0.5f), 0.0f, 1.0f, GBuffer);

        out_GBuffer0 = GBuffer.m_Color0;
        out_GBuffer1 = GBuffer.m_Color1;
        out_GBuffer2 = GBuffer.m_Color2;

        vec4 CSPosition = g_WorldToScreen * vec4(Vertex, 1.0f);
        gl_FragDepth = (CSPosition.z / CSPosition.w);

        return;
    }

    discard;
}

#endif // __INCLUDE_FS_RAYCAST_GLSL__