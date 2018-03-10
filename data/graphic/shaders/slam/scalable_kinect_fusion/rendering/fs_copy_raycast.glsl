
#ifndef __INCLUDE_FS_RAYCAST_GLSL__
#define __INCLUDE_FS_RAYCAST_GLSL__

#include "common_global.glsl"
#include "common_gbuffer.glsl"
#include "slam/scalable_kinect_fusion/common_raycast.glsl"

layout(std140, binding = 1) uniform PerDrawCallData
{
    vec4 g_LightPosition;
    vec4 g_Color;
};

layout (binding = 0, rgba16f) uniform image2D fs_Intermediate0;
layout (binding = 1, rgba16f) uniform image2D fs_Intermediate1;

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
    vec3 Color = imageLoad(fs_Intermediate0, ivec2(gl_FragCoord.xy)).xyz;
    vec3 WSPosition = imageLoad(fs_Intermediate1, ivec2(gl_FragCoord.xy)).xyz;

    if (WSPosition.x != 0.0f)
    {
        SGBuffer GBuffer;

        PackGBuffer(vec3(0.0f, 1.0f, 0.0f), vec3(1.0f, 0.0f, 0.0f), 0.5f, vec3(0.5f), 0.0f, 1.0f, GBuffer);

        out_GBuffer0 = GBuffer.m_Color0;
        out_GBuffer1 = GBuffer.m_Color1;
        out_GBuffer2 = GBuffer.m_Color2;

        vec4 CSPosition = g_WorldToScreen * vec4(WSPosition, 1.0f);
        gl_FragDepth = (CSPosition.z / CSPosition.w) * 0.5f + 0.5f;

        return;
    }

    //discard;
}

#endif // __INCLUDE_FS_RAYCAST_GLSL__