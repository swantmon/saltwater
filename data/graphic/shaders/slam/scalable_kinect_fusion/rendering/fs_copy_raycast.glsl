
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

vec3 GetNormal(ivec2 Position)
{
    vec3 Vertex0 = imageLoad(fs_Intermediate1, Position + ivec2(0, 0)).xyz;
    vec3 Vertex1 = imageLoad(fs_Intermediate1, Position + ivec2(1, 0)).xyz;
    vec3 Vertex2 = imageLoad(fs_Intermediate1, Position + ivec2(0, 1)).xyz;
    
    return normalize(cross(Vertex1 - Vertex0, Vertex2 - Vertex0));
}

vec3 GetSmoothNormal(ivec2 Position)
{
    vec3 Sum = vec3(0.0f);
    int Count = 0;
    
    for (int i = -2; i <= 2; ++ i)
    {
        for (int j = -2; j <= 2; ++ j)
        {
            ++ Count;
            Sum += GetNormal(Position += ivec2(i, j));
        }
    }
    
    return Sum / Count;
}

void main()
{
    vec3 Color = imageLoad(fs_Intermediate0, ivec2(gl_FragCoord.xy)).xyz;
    vec3 WSPosition = imageLoad(fs_Intermediate1, ivec2(gl_FragCoord.xy)).xyz;
    
    vec3 Normal = GetSmoothNormal(ivec2(gl_FragCoord.xy));
    
    if (WSPosition.x != 0.0f)
    {
        SGBuffer GBuffer;

        PackGBuffer(Color, Normal, 0.5f, vec3(0.5f), 0.0f, 1.0f, GBuffer);

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