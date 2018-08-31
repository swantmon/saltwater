
#ifndef __INCLUDE_FS_RAYCAST_GLSL__
#define __INCLUDE_FS_RAYCAST_GLSL__
 
#include "common_global.glsl"
#include "common_gbuffer.glsl"
#include "slam/scalable_kinect_fusion/common_raycast.glsl"
 
layout(std140, binding = 1) uniform PerDrawCallData
{
    mat4 g_WSToSelectionTransform;
};

bool IsInBox(vec3 Position)
{
    Position = (g_WSToSelectionTransform * vec4(Position, 1.0f)).xyz;
    bool IsInX = Position.x > 0.0f && Position.x < 1.0f;
    bool IsInY = Position.y > 0.0f && Position.y < 1.0f;
    bool IsInZ = Position.z > 0.0f && Position.z < 1.0f;
    return IsInX && IsInY && IsInZ;
}

vec3 GetPositionHightlight(vec3 CameraPosition, vec3 RayDirection)
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
        else if (CurrentTSDF * PreviousTSDF < 0.0f && !IsInBox(PreviousPosition))
#else
        else if (CurrentTSDF < 0.0f && PreviousTSDF > 0.0f && !IsInBox(PreviousPosition))
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

void GetPositionAndColorHighlight(vec3 CameraPosition, vec3 RayDirection, out vec3 Vertex, out vec3 Color)
{
    Vertex = GetPositionHightlight(CameraPosition, RayDirection);
#ifdef CAPTURE_COLOR
    Color = GetColor(Vertex);
#else
    Color = vec3(1.0f);
#endif
}

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
 
    vec3 Cameraposition = SaltwaterToReconstruction * g_ViewPosition.xyz;
    vec3 RayDirection = SaltwaterToReconstruction * normalize(in_WSRayDirection);
 
    RayDirection.x = RayDirection.x == 0.0f ? 1e-15f : RayDirection.x;
    RayDirection.y = RayDirection.y == 0.0f ? 1e-15f : RayDirection.y;
    RayDirection.z = RayDirection.z == 0.0f ? 1e-15f : RayDirection.z;
 
    vec3 WSPosition, Color;

    GetPositionAndColorHighlight(Cameraposition, RayDirection, WSPosition, Color);

    if (WSPosition.x != 0.0f && (Color.r != 0.0f || Color.g != 0.0f || Color.b != 0.0f))
    {
        vec3 WSNormal = mat3(ReconstructionToSaltwater) * GetNormal(WSPosition);
        WSNormal *= -1.0f;

        SGBuffer GBuffer;
 
        PackGBuffer(Color, WSNormal, 0.5f, vec3(0.5f), 0.0f, 1.0f, GBuffer);
 
        out_GBuffer0 = GBuffer.m_Color0;
        out_GBuffer1 = GBuffer.m_Color1;
        out_GBuffer2 = GBuffer.m_Color2;
 
        vec4 CSPosition = g_WorldToScreen * vec4(ReconstructionToSaltwater * WSPosition, 1.0f);
        gl_FragDepth = (CSPosition.z / CSPosition.w) * 0.5f + 0.5f;
 
        return;
    }
 
    discard;
}

#endif // __INCLUDE_FS_RAYCAST_GLSL__