
#ifndef __INCLUDE_FS_RAYCAST_DIMINISHED_GLSL__
#define __INCLUDE_FS_RAYCAST_DIMINISHED_GLSL__
 
#include "common_global.glsl"
#include "slam/scalable_kinect_fusion/common_raycast.glsl"

// -----------------------------------------------------------------------------
// Data
// ----------------------------------------------------------------------------- 
layout(std140, binding = 1) uniform PerDrawCallData
{
    mat4 g_WSToSelectionTransform;
};

layout(std140, binding = 3) uniform UBOIntrinsics
{
    mat4 m_KMatrix;
    mat4 m_InvKMatrix;
    vec2 m_FocalPoint;
    vec2 m_FocalLength;
    vec2 m_InvFocalLength;
};

// -----------------------------------------------------------------------------
// functions
// -----------------------------------------------------------------------------
bool IsInBox(vec3 Position)
{
    Position = (g_WSToSelectionTransform * vec4(Position, 1.0f)).xyz;
    bool IsInX = Position.x > 0.0f && Position.x < 1.0f;
    bool IsInY = Position.y > 0.0f && Position.y < 1.0f;
    bool IsInZ = Position.z > 0.0f && Position.z < 1.0f;
    return IsInX && IsInY && IsInZ;
}

// -----------------------------------------------------------------------------

bool IsInBox2(vec3 Position)
{
    Position = (g_WSToSelectionTransform * vec4(Position, 1.0f)).xyz;
    bool IsInX = Position.x > 0.0f && Position.x < 1.0f;
    bool IsInY = Position.y > 0.0f && Position.y < 1.0f;
    bool IsInZ = Position.z > 0.1f && Position.z < 1.0f;
    return IsInX && IsInY && IsInZ;
}

// -----------------------------------------------------------------------------

vec3 GetDiminishedPosition(vec3 CameraPosition, vec3 RayDirection)
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
#ifdef RAYCAST_BACKSIDES
        else if (CurrentTSDF * PreviousTSDF < 0.0f && !IsInBox2(CurrentPosition))
#else
        else if (CurrentTSDF < 0.0f && PreviousTSDF > 0.0f && !IsInBox2(CurrentPosition))
#endif
        {
            break;
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

    return Vertex;
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

    vec3 VertexPixelPosition;
    VertexPixelPosition.xy = vec2(gl_FragCoord.xy - m_FocalPoint) * m_InvFocalLength;
    VertexPixelPosition.z = 1.0f;
 
    vec3 CameraPosition = SaltwaterToReconstruction * g_ViewPosition.xyz;
    vec3 RayDirection = normalize(VertexPixelPosition);
    //vec3 RayDirection = SaltwaterToReconstruction * normalize(in_WSRayDirection);
 
    RayDirection.x = RayDirection.x == 0.0f ? 1e-15f : RayDirection.x;
    RayDirection.y = RayDirection.y == 0.0f ? 1e-15f : RayDirection.y;
    RayDirection.z = RayDirection.z == 0.0f ? 1e-15f : RayDirection.z;
 
    vec3 WSPosition, Color, Vertex;

    Vertex = GetDiminishedPosition(CameraPosition, RayDirection);

#ifdef CAPTURE_COLOR
    Color = GetColor(Vertex);
#else
    Color = vec3(1.0f);
#endif
    
    //vec4 FinalColor = IsInBox(Vertex) ? vec4(Color, 1.0f) : vec4(0.0f);
    vec4 FinalColor = vec4(Color, 1.0f);

    out_DiminishedColor = FinalColor;
}

#endif // __INCLUDE_FS_RAYCAST_DIMINISHED_GLSL__