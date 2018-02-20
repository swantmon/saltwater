
#ifndef __INCLUDE_COMMON_RAYCAST_GLSL__
#define __INCLUDE_COMMON_RAYCAST_GLSL__

#include "slam/scalable_kinect_fusion/common_scalable.glsl"

// -----------------------------------------------------------------------------
// Input from engine
// -----------------------------------------------------------------------------

layout(std140, binding = 2) uniform ScalableRaycastConstantBuffer
{
    vec3 g_AABBMin;
    vec3 g_AABBMax;
    int g_VolumeTextureWidth;
};

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
    ivec3 ItemOffset = ivec3(floor(PositionInVolume * ROOT_RESOLUTION));
    ivec3 VolumeOffset = ItemOffset % ROOT_RESOLUTION;
    
    int BufferOffset = VolumeOffset.z * ROOT_RESOLUTION * ROOT_RESOLUTION;
    BufferOffset += VolumeOffset.y * ROOT_RESOLUTION + VolumeOffset.x;
    
    return VolumeBufferOffset * VOXELS_PER_ROOTGRID + BufferOffset;
}

uint GetRawVoxel(vec3 Position)
{
    Position /= VOLUME_SIZE;

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
                ivec3 Level1VolumeOffset = ivec3(floor(Position * ROOT_RESOLUTION * LEVEL1_RESOLUTION));
                Level1VolumeOffset %= LEVEL1_RESOLUTION;

                int Level1BufferInnerOffset = OffsetToIndex(Level1VolumeOffset, LEVEL1_RESOLUTION);
                int Level1BufferIndex = Level1VolumeBufferOffset * VOXELS_PER_LEVEL1GRID + Level1BufferInnerOffset;

                int TSDFVolumeBufferOffset = g_Level1GridPool[Level1BufferIndex].m_PoolIndex;

                if (TSDFVolumeBufferOffset != -1)
                {
                    float TotalResolution = ROOT_RESOLUTION * LEVEL1_RESOLUTION * LEVEL2_RESOLUTION;

                    ivec3 TSDFVolumeOffset = ivec3(floor(Position * TotalResolution));
                    TSDFVolumeOffset %= 8;

                    int TSDFBufferInnerOffset = OffsetToIndex(TSDFVolumeOffset, LEVEL2_RESOLUTION);
                    int TSDFBufferIndex = TSDFVolumeBufferOffset * VOXELS_PER_LEVEL2GRID + TSDFBufferInnerOffset;

                    return g_TSDFPool[TSDFBufferIndex];
                }
            }
        }
    }
    return 0;
}

vec2 GetVoxel(vec3 Position)
{
    return UnpackVoxel(GetRawVoxel(Position));
}

vec2 GetVoxelWithStep(vec3 Position, vec3 Direction, out float Step)
{
    Step = -1.0f;   
    Position /= VOLUME_SIZE;

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
                ivec3 Level1VolumeOffset = ivec3(floor(Position * ROOT_RESOLUTION * LEVEL1_RESOLUTION));
                Level1VolumeOffset %= 8;

                int Level1BufferInnerOffset = OffsetToIndex(Level1VolumeOffset, LEVEL1_RESOLUTION);
                int Level1BufferIndex = Level1VolumeBufferOffset * VOXELS_PER_LEVEL1GRID + Level1BufferInnerOffset;

                int TSDFVolumeBufferOffset = g_Level1GridPool[Level1BufferIndex].m_PoolIndex;

                if (TSDFVolumeBufferOffset != -1)
                {
                    float TotalResolution = ROOT_RESOLUTION * LEVEL1_RESOLUTION * LEVEL2_RESOLUTION;

                    ivec3 TSDFVolumeOffset = ivec3(floor(Position * TotalResolution));
                    TSDFVolumeOffset %= LEVEL2_RESOLUTION;

                    int TSDFBufferInnerOffset = OffsetToIndex(TSDFVolumeOffset, LEVEL2_RESOLUTION);
                    int TSDFBufferIndex = TSDFVolumeBufferOffset * VOXELS_PER_LEVEL2GRID + TSDFBufferInnerOffset;
                    
                    return UnpackVoxel(g_TSDFPool[TSDFBufferIndex]);
                }
                else
                {
                    Position *= VOLUME_SIZE;
            
                    vec3 AABBMin = Position - mod(Position, VOLUME_SIZE / (ROOT_RESOLUTION * LEVEL1_RESOLUTION));
                    vec3 AABBMax = AABBMin + VOLUME_SIZE / (ROOT_RESOLUTION * LEVEL1_RESOLUTION);
            
                    Step = GetEndLength(Position, Direction, AABBMin, AABBMax);
                }
            }
            else
            {
                Position *= VOLUME_SIZE;
            
                vec3 AABBMin = Position - mod(Position, VOLUME_SIZE / (ROOT_RESOLUTION));
                vec3 AABBMax = AABBMin + VOLUME_SIZE / (ROOT_RESOLUTION);
            
                Step = GetEndLength(Position, Direction, AABBMin, AABBMax);
            }
        }        
        else
        {
            Position *= VOLUME_SIZE;
            
            vec3 AABBMin = Position - mod(Position, VOLUME_SIZE);
            vec3 AABBMax = AABBMin + VOLUME_SIZE;
            
            Step = GetEndLength(Position, Direction, AABBMin, AABBMax);
        }
    }    
    else
    {
        Position *= VOLUME_SIZE;
                
        vec3 AABBMin = Position - mod(Position, VOLUME_SIZE);
        vec3 AABBMax = AABBMin + VOLUME_SIZE;
                
        Step = GetEndLength(Position, Direction, AABBMin, AABBMax);
    }
    
    return vec2(0.0f);
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

    g = g * VOXEL_SIZE;

    const float result =
    GetVoxel(vec3(g.x             , g.y             , g.z             )).x * (1.0f - a) * (1.0f - b) * (1.0f - c) +
    GetVoxel(vec3(g.x             , g.y             , g.z + VOXEL_SIZE)).x * (1.0f - a) * (1.0f - b) *         c  +
    GetVoxel(vec3(g.x             , g.y + VOXEL_SIZE, g.z             )).x * (1.0f - a) *         b  * (1.0f - c) +
    GetVoxel(vec3(g.x             , g.y + VOXEL_SIZE, g.z + VOXEL_SIZE)).x * (1.0f - a) *         b  *         c  +
    GetVoxel(vec3(g.x + VOXEL_SIZE, g.y             , g.z             )).x *         a  * (1.0f - b) * (1.0f - c) +
    GetVoxel(vec3(g.x + VOXEL_SIZE, g.y             , g.z + VOXEL_SIZE)).x *         a  * (1.0f - b) *         c  +
    GetVoxel(vec3(g.x + VOXEL_SIZE, g.y + VOXEL_SIZE, g.z             )).x *         a  *         b  * (1.0f - c) +
    GetVoxel(vec3(g.x + VOXEL_SIZE, g.y + VOXEL_SIZE, g.z + VOXEL_SIZE)).x *         a  *         b  *         c;

    return result;
}

#ifdef CAPTURE_COLOR

vec3 GetColor(vec3 Position)
{
    vec3 g = ivec3(floor(Position / VOXEL_SIZE));

    const float vx = (g.x + 0.5f) * VOXEL_SIZE;
    const float vy = (g.y + 0.5f) * VOXEL_SIZE;
    const float vz = (g.z + 0.5f) * VOXEL_SIZE;

    g.x = (Position.x < vx) ? (g.x - 1) : g.x;
    g.y = (Position.y < vy) ? (g.y - 1) : g.y;
    g.z = (Position.z < vz) ? (g.z - 1) : g.z;

    g = g * VOXEL_SIZE;

    uint Voxel = GetRawVoxel(g);

    vec3 Color;
    vec2 Unused = UnpackVoxel(Voxel, Color);

    return Color;
}

#endif

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

vec3 GetPosition(vec3 CameraPosition, vec3 RayDirection)
{
    const float TruncatedDistance = TRUNCATED_DISTANCE / 1000.0f;

    const float StartLength = GetStartLength(CameraPosition, RayDirection, g_AABBMin, g_AABBMax);
    const float EndLength = GetEndLength(CameraPosition, RayDirection, g_AABBMin, g_AABBMax);

    float RayLength = StartLength;
    float Step = TruncatedDistance;

    float CurrentTSDF = GetVoxel(CameraPosition + RayLength * RayDirection).x;
    float PreviousTSDF;
    RayLength += Step;

    vec3 Vertex = vec3(0.0f);
        
    float NewStep;
    
    while (RayLength < EndLength)
    {
        vec3 PreviousPosition = CameraPosition + RayLength * RayDirection;
        RayLength += Step;
        vec3 CurrentPosition = CameraPosition + RayLength * RayDirection;

        PreviousTSDF = CurrentTSDF;
        
        CurrentTSDF = GetVoxelWithStep(CurrentPosition, RayDirection, NewStep).x;

        if (NewStep > 0.0f)
        {
            RayLength += NewStep;
        }
        else if (CurrentTSDF < 0.0f && PreviousTSDF > 0.0f)
        {
            float Ft = GetInterpolatedTSDF(PreviousPosition);
            float Ftdt = GetInterpolatedTSDF(CurrentPosition);
            float Ts = RayLength - Step * Ft / (Ftdt - Ft);

            Vertex = CameraPosition + RayDirection * Ts;

            break;
        }
        
        Step = CurrentTSDF < 1.0f ? VOXEL_SIZE : TruncatedDistance;
    }

    return Vertex;
}

#ifdef CAPTURE_COLOR

void GetPositionAndColor(vec3 CameraPosition, vec3 RayDirection, out vec3 Vertex, out vec3 Color)
{
    const float TruncatedDistance = TRUNCATED_DISTANCE / 1000.0f;

    const float StartLength = GetStartLength(CameraPosition, RayDirection, g_AABBMin, g_AABBMax);
    const float EndLength = GetEndLength(CameraPosition, RayDirection, g_AABBMin, g_AABBMax);

    float RayLength = StartLength;
    float Step = TruncatedDistance;

    float CurrentTSDF = GetVoxel(CameraPosition + RayLength * RayDirection).x;
    float PreviousTSDF;
    RayLength += Step;

    Vertex = vec3(0.0f);
    Color = vec3(0.0f);

    while (RayLength < EndLength)
    {
        vec3 PreviousPosition = CameraPosition + RayLength * RayDirection;
        RayLength += Step;
        vec3 CurrentPosition = CameraPosition + RayLength * RayDirection;

        PreviousTSDF = CurrentTSDF;
        CurrentTSDF = GetVoxel(CurrentPosition).x;

        if (CurrentTSDF < 0.0f && PreviousTSDF > 0.0f)
        {
            float Ft = GetInterpolatedTSDF(PreviousPosition);
            float Ftdt = GetInterpolatedTSDF(CurrentPosition);
            float Ts = RayLength - Step * Ft / (Ftdt - Ft);

            Vertex = CameraPosition + RayDirection * Ts;
            Color = GetColor(PreviousPosition);

            break;
        }
        
        Step = CurrentTSDF < 1.0f ? VOXEL_SIZE : TruncatedDistance;
    }
}


#endif

#endif // __INCLUDE_COMMON_RAYCAST_GLSL__