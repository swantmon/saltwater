
#ifndef __INCLUDE_FS_RAYCAST_DIMINISHED_GLSL__
#define __INCLUDE_FS_RAYCAST_DIMINISHED_GLSL__
 
#include "common_global.glsl"
#include "../../plugins/slam/scalable/common_raycast.glsl"

// -----------------------------------------------------------------------------
// Data
// ----------------------------------------------------------------------------- 
layout(std140, binding = 1) uniform PerDrawCallData
{
    mat4 g_WSToSelectionTransform;
};

// -----------------------------------------------------------------------------
// functions
// -----------------------------------------------------------------------------
bool IsInBox(vec3 Position)
{
    Position = (g_WSToSelectionTransform * vec4(Position, 1.0f)).xyz;
    bool IsInX = Position.x > 0.0f && Position.x < 1.0f;
    bool IsInY = Position.y > 0.0f && Position.y < 1.0f;
    bool IsInZ = Position.z > 0.0f && Position.z < 2.0f;
    return IsInX && IsInY && IsInZ;
}

// -----------------------------------------------------------------------------

vec4 GetDiminishedPosition(vec3 CameraPosition, vec3 RayDirection)
{
    const float StartLength = max(RAYCAST_NEAR, GetStartLength(CameraPosition, RayDirection, g_AABBMin, g_AABBMax));
    const float EndLength = min(RAYCAST_FAR,GetEndLength(CameraPosition, RayDirection, g_AABBMin, g_AABBMax));

    float RayLength = StartLength;
    float Step = TRUNCATED_DISTANCE;

    float CurrentTSDF = GetVoxel(CameraPosition + RayLength * RayDirection).x;
    float PreviousTSDF;
    RayLength += Step;

    vec3 Vertex = vec3(0.0f);

    float NewStep;
    
    vec3 PreviousPosition;
    vec3 CurrentPosition;

    bool hit = false;

    while (RayLength < EndLength)
    {
        PreviousPosition = CameraPosition + RayLength * RayDirection;
        RayLength += Step;
        CurrentPosition = CameraPosition + RayLength * RayDirection;

        PreviousTSDF = CurrentTSDF;
        
        CurrentTSDF = GetVoxelWithStep(CurrentPosition, RayDirection, NewStep).x;

        if (NewStep > 0.0f)
        {
            RayLength += NewStep;
        }
        else if (CurrentTSDF < 0.0f && PreviousTSDF > 0.0f && !IsInBox(CurrentPosition))
        {
            break;
        }
        else if (CurrentTSDF < 0.0f && PreviousTSDF > 0.0f)
        {
            hit = true;
        }
        
        //Step = CurrentTSDF < 1.0f ? VOXEL_SIZE : TRUNCATED_DISTANCE;
        Step = VOXEL_SIZE;
    }

    if (RayLength < EndLength)
    {
        float Ft = GetInterpolatedTSDF(PreviousPosition);
        float Ftdt = GetInterpolatedTSDF(CurrentPosition);
        float Ts = RayLength - Step * Ft / (Ftdt - Ft);

        Vertex = CameraPosition + RayDirection * Ts;
    }

    return hit ? vec4(Vertex, 1.0f) : vec4(Vertex, 0.0f);
}

// -----------------------------------------------------------------------------
// Input from previous shader stage
// -----------------------------------------------------------------------------
 
layout(location = 0) in vec3 in_WSRayDirection;
 
layout(location = 0) out vec4 out_DiminishedColor;
 
// -----------------------------------------------------------------------------
// Main
// -----------------------------------------------------------------------------
void main()
{
    mat3 SaltwaterToReconstruction = mat3(
        1.0f, 0.0f,  0.0f,
        0.0f, 0.0f, -1.0f,
        0.0f, 1.0f,  0.0f
    );
 
    mat3 ReconstructionToSaltwater = mat3(
        1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f,
        0.0f, -1.0f, 0.0f
    );
 
    vec3 CameraPosition = SaltwaterToReconstruction * g_ViewPosition.xyz;
    vec3 RayDirection = SaltwaterToReconstruction * normalize(in_WSRayDirection);
 
    RayDirection.x = RayDirection.x == 0.0f ? 1e-15f : RayDirection.x;
    RayDirection.y = RayDirection.y == 0.0f ? 1e-15f : RayDirection.y;
    RayDirection.z = RayDirection.z == 0.0f ? 1e-15f : RayDirection.z;

    vec4 Vertex = GetDiminishedPosition(CameraPosition, RayDirection);

    if (Vertex.x == 0.0f)
    {
        discard;
    }
    
    vec3 Color = GetColor(Vertex.xyz);
    
    out_DiminishedColor = vec4(Color, Vertex.w);
}

#endif // __INCLUDE_FS_RAYCAST_DIMINISHED_GLSL__