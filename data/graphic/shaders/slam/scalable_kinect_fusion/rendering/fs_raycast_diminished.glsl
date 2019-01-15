
#ifndef __INCLUDE_FS_RAYCAST_DIMINISHED_GLSL__
#define __INCLUDE_FS_RAYCAST_DIMINISHED_GLSL__
 
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

void GetPositionAndColorHighlight(vec3 CameraPosition, vec3 RayDirection, out vec3 Vertex, out vec3 Color)
{
    Vertex = GetPosition(CameraPosition, RayDirection);
#ifdef CAPTURE_COLOR
    Color = GetColor(Vertex);
#else
    Color = vec3(1.0f);
#endif
    Color = IsInBox(Vertex) ? (Color + vec3(0.0f, 0.0f, 1.0f)) / 2.0f : Color;
}

// -----------------------------------------------------------------------------
// Input from previous shader stage
// -----------------------------------------------------------------------------
 
layout(location = 0) in vec3 in_WSRayDirection;
 
layout(location = 0) out vec4 out_DiminishedColor;
 
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
 
    vec3 CameraPosition = SaltwaterToReconstruction * g_ViewPosition.xyz;
    vec3 RayDirection = SaltwaterToReconstruction * normalize(in_WSRayDirection);
 
    RayDirection.x = RayDirection.x == 0.0f ? 1e-15f : RayDirection.x;
    RayDirection.y = RayDirection.y == 0.0f ? 1e-15f : RayDirection.y;
    RayDirection.z = RayDirection.z == 0.0f ? 1e-15f : RayDirection.z;
 
    vec3 WSPosition, Color, Vertex;

    Vertex = GetPosition(CameraPosition, RayDirection);

#ifdef CAPTURE_COLOR
    Color = GetColor(Vertex);
#else
    Color = vec3(1.0f);
#endif
    
    vec4 FinalColor = IsInBox(Vertex) ? vec4(Color, 1.0f) : vec4(0.0f);

    if (WSPosition.x != 0.0f && (Color.r != 0.0f || Color.g != 0.0f || Color.b != 0.0f))
    {
        out_DiminishedColor = FinalColor;
    }
 
    out_DiminishedColor = vec4(0.0f);
}

#endif // __INCLUDE_FS_RAYCAST_DIMINISHED_GLSL__