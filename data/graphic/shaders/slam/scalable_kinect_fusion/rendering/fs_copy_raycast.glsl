
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

layout(location = 0) out vec4 out_GBuffer0;
layout(location = 1) out vec4 out_GBuffer1;
layout(location = 2) out vec4 out_GBuffer2;

// -----------------------------------------------------------------------------
// Helper functions
// -----------------------------------------------------------------------------

void main()
{
    ivec2 xy = ivec2(gl_FragCoord.xy);
    
    vec4 Data0 = imageLoad(fs_Intermediate0, xy);
    vec4 Data1 = imageLoad(fs_Intermediate1, xy);

    if (Data0.w > 0.0f)
    {
        vec3 WSPosition = Data1.xyz;

        vec3 Normal = Data0.xyz;
                
        SGBuffer GBuffer;

        PackGBuffer(vec3(0.0f, 1.0f, 0.0f), Normal, 0.5f, vec3(0.5f), 0.0f, 1.0f, GBuffer);

        out_GBuffer0 = GBuffer.m_Color0;
        out_GBuffer1 = GBuffer.m_Color1;
        out_GBuffer2 = GBuffer.m_Color2;

        vec4 CSPosition = g_WorldToScreen * vec4(WSPosition, 1.0f);
        gl_FragDepth = (CSPosition.z / CSPosition.w) * 0.5f + 0.5f;

        return;
    }

    discard;
}

#endif // __INCLUDE_FS_RAYCAST_GLSL__