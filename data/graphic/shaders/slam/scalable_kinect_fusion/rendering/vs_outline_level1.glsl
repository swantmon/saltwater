
#ifndef __INCLUDE_VS_OUTLINE_GLSL__
#define __INCLUDE_VS_OUTLINE_GLSL__

#include "common_global.glsl"
#include "slam/scalable_kinect_fusion/common_scalable.glsl"

layout(std140, binding = 1) uniform PerDrawCallData
{
    mat4 g_WorldMatrix;
	vec4 g_Color;
};

layout(std430, binding = 2) buffer VolumeQueue
{
    uint g_VolumeID[];
};

layout(location = 0) in vec3 in_VertexPosition;

out gl_PerVertex
{
    vec4 gl_Position;
};

// -----------------------------------------------------------------------------
// Functions
// -----------------------------------------------------------------------------

void main()
{
    vec3 Offset = IndexToOffset(int(g_VolumeID[gl_InstanceID]), ROOT_RESOLUTION);
    vec4 WSPosition = g_WorldMatrix * vec4(in_VertexPosition + Offset, 1.0f);
    gl_Position = g_WorldToScreen * vec4(WSPosition.xyz, 1.0f);
}

#endif // __INCLUDE_VS_OUTLINE_GLSL__