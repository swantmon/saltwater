
#ifndef __INCLUDE_FS_KINECT_RAYCAST_GLSL__
#define __INCLUDE_FS_KINECT_RAYCAST_GLSL__

#include "common_global.glsl"
#include "common_raycast.glsl"
#include "common_gbuffer.glsl"

// -----------------------------------------------------------------------------
// Constants
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Input from engine
// -----------------------------------------------------------------------------

layout(row_major, std140, binding = 1) uniform PerDrawCallData
{
    vec4 g_LightPosition;
    vec4 g_Color;
};

layout (binding = 0) uniform isampler3D fs_Volume;

layout(location = 0) in vec3 in_WSRayDirection;

layout(location = 0) out vec4 out_GBuffer0;
layout(location = 1) out vec4 out_GBuffer1;
layout(location = 2) out vec4 out_GBuffer2;

void main()
{
    vec3 RayDirection = normalize(in_WSRayDirection);

    RayDirection.x = RayDirection.x == 0.0f ? 1e-15f : RayDirection.x;
    RayDirection.y = RayDirection.y == 0.0f ? 1e-15f : RayDirection.y;
    RayDirection.z = RayDirection.z == 0.0f ? 1e-15f : RayDirection.z;

    vec3 WSPosition = GetPosition(g_ViewPosition.xyz, RayDirection, fs_Volume);
    
    if (WSPosition.x != 0.0f)
    {
        vec3 WSNormal = GetNormal(WSPosition, fs_Volume);
        
        WSNormal.x = -WSNormal.x;
		WSNormal.z = -WSNormal.z;
        
        SGBuffer GBuffer;

        PackGBuffer(g_Color.rgb, WSNormal, 0.5f, vec3(0.5f), 0.0f, 1.0f, GBuffer);

        out_GBuffer0 = GBuffer.m_Color0;
        out_GBuffer1 = GBuffer.m_Color1;
        out_GBuffer2 = GBuffer.m_Color2;

        vec4 CSPosition = g_WorldToScreen * vec4(WSPosition, 1.0f);
        gl_FragDepth = (CSPosition.z / CSPosition.w) * 0.5f + 0.5f;
    }
    else
    {
        discard;
    }
}

#endif // __INCLUDE_FS_KINECT_RAYCAST_GLSL__