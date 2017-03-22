
#ifndef __INCLUDE_COMMON_RAYCAST_GLSL__
#define __INCLUDE_COMMON_RAYCAST_GLSL__

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

ivec3 GetVoxelCoords(vec3 Position)
{
    return ivec3(Position / VOXEL_SIZE);
}

vec2 GetVoxel(ivec3 Coords, isampler3D Volume)
{
    vec2 Voxel = texelFetch(Volume, Coords, 0).xy;
    Voxel.x /= float(INT16_MAX);
    return Voxel;
}

float GetInterpolatedTSDF(vec3 Position, isampler3D Volume)
{
    vec3 Coords = GetVoxelCoords(Position);

    return textureLod(Volume, Coords / float(VOLUME_RESOLUTION), 0).x / float(INT16_MAX);
}

vec3 GetPosition(vec3 CameraPosition, vec3 RayDirection, isampler3D Volume)
{

    const float StartLength = GetStartLength(CameraPosition, RayDirection);
    const float EndLength = GetEndLength(CameraPosition, RayDirection);
    
    float Step = TRUNCATED_DISTANCE * 0.001f * 0.8f;
    float RayLength = StartLength;

    vec2 Voxel = GetVoxel(GetVoxelCoords(CameraPosition + RayLength * RayDirection), Volume);

    float TSDF = Voxel.x;

    vec3 Vertex = vec3(0.0f);

    while (RayLength <= EndLength)
    {
        vec3 PreviousPosition = CameraPosition + RayLength * RayDirection;
        RayLength += Step;
        vec3 CurrentPosition = CameraPosition + RayLength * RayDirection;

        float PreviousTSDF = TSDF;

        ivec3 VoxelCoords = GetVoxelCoords(CurrentPosition);

        vec2 Voxel = GetVoxel(VoxelCoords, Volume);

        TSDF = Voxel.x;
        
        if (PreviousTSDF > 0.0f && TSDF < 0.0f)
        {
            float Ft = GetInterpolatedTSDF(PreviousPosition, Volume);
            float Ftdt = GetInterpolatedTSDF(CurrentPosition, Volume);
            float Ts = RayLength - Step * Ft / (Ftdt - Ft);

            Vertex = CameraPosition + RayDirection * Ts;
                        
            break;
        }

        if (TSDF < 1.0f)
        {
            Step = VOXEL_SIZE;
        }
    }

    return Vertex;
}

vec3 GetNormal(vec3 Vertex, isampler3D Volume)
{
    vec3 T, Normal;

    T = Vertex;
    T.x += VOXEL_SIZE;
    float Fx1 = GetInterpolatedTSDF(T, Volume);
    T = Vertex;
    T.x -= VOXEL_SIZE;
    float Fx2 = GetInterpolatedTSDF(T, Volume);

    T = Vertex;
    T.y += VOXEL_SIZE;
    float Fy1 = GetInterpolatedTSDF(T, Volume);
    T = Vertex;
    T.y -= VOXEL_SIZE;
    float Fy2 = GetInterpolatedTSDF(T, Volume);

    T = Vertex;
    T.z += VOXEL_SIZE;
    float Fz1 = GetInterpolatedTSDF(T, Volume);
    T = Vertex;
    T.z -= VOXEL_SIZE;
    float Fz2 = GetInterpolatedTSDF(T, Volume);

    Normal.x = Fx2 - Fx1;
    Normal.y = Fy2 - Fy1;
    Normal.z = Fz2 - Fz1;

    return normalize(Normal);
}

#endif // __INCLUDE_COMMON_RAYCAST_GLSL__