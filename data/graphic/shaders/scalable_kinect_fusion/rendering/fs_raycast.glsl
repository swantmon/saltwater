
#ifndef __INCLUDE_FS_KINECT_RAYCAST_GLSL__
#define __INCLUDE_FS_KINECT_RAYCAST_GLSL__

#include "common_global.glsl"
#include "scalable_kinect_fusion/common_raycast.glsl"
#include "scalable_kinect_fusion/common_scalable.glsl"
#include "common_gbuffer.glsl"

// -----------------------------------------------------------------------------
// Constants
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Input from engine
// -----------------------------------------------------------------------------

layout(row_major, std140, binding = 1) uniform PerDrawCallData
{
    vec4 g_Offset;
    vec4 g_Color;
};

layout(location = 0) in vec3 in_WSRayDirection;

layout(location = 0) out vec4 out_GBuffer0;
layout(location = 1) out vec4 out_GBuffer1;
layout(location = 2) out vec4 out_GBuffer2;

void main()
{
    out_GBuffer0 = vec4(0.0f, 0.0f, 0.0f, 1.0f);
    out_GBuffer1 = vec4(0.0f, 0.0f, 0.0f, 1.0f);
    out_GBuffer2 = vec4(0.0f, 0.0f, 0.0f, 1.0f);
}

#endif // __INCLUDE_FS_KINECT_RAYCAST_GLSL__