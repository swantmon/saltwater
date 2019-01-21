
#ifndef __INCLUDE_FS_RAYCAST_HITPROXY_GLSL__
#define __INCLUDE_FS_RAYCAST_HITPROXY_GLSL__
 
#include "common_global.glsl"
#include "common_gbuffer.glsl"
#include "slam/scalable_kinect_fusion/common_raycast.glsl"
 
layout(std140, binding = 1) uniform UB1
{
    uint in_EntityID;
};
 
// -----------------------------------------------------------------------------
// Input from previous shader stage
// -----------------------------------------------------------------------------
 
layout(location = 0) in vec3 in_WSRayDirection;
 
layout(location = 0) out uint out_HitProxy;
 
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
 
    vec3 Cameraposition = SaltwaterToReconstruction * g_ViewPosition.xyz;
    vec3 RayDirection = SaltwaterToReconstruction * normalize(in_WSRayDirection);
 
    RayDirection.x = RayDirection.x == 0.0f ? 1e-15f : RayDirection.x;
    RayDirection.y = RayDirection.y == 0.0f ? 1e-15f : RayDirection.y;
    RayDirection.z = RayDirection.z == 0.0f ? 1e-15f : RayDirection.z;
 
    vec3 WSPosition = GetPosition(Cameraposition, RayDirection);
 
    if (WSPosition.x != 0.0f)
    {
        out_HitProxy = in_EntityID;

        vec4 CSPosition = g_WorldToScreen * vec4(ReconstructionToSaltwater * WSPosition, 1.0f);
        gl_FragDepth = (CSPosition.z / CSPosition.w) * 0.5f + 0.5f;
 
        return;
    }
 
    discard;
}


#endif // __INCLUDE_FS_RAYCAST_HITPROXY_GLSL__