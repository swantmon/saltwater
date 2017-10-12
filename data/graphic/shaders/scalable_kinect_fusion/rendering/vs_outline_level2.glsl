
#ifndef __INCLUDE_VS_OUTLINE_GLSL__
#define __INCLUDE_VS_OUTLINE_GLSL__

#include "common_global.glsl"
#include "scalable_kinect_fusion/common_scalable.glsl"

layout(row_major, std140, binding = 1) uniform PerDrawCallData
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

#ifdef FULL_VOLUME_INTEGRATION

void main()
{
    uint VolumeID = g_VolumeID[gl_InstanceID];

    vec3 Offset = IndexToOffset(VolumeID, 16 * 8);

    Offset = Offset * (0.004 * 8);

    vec4 WSPosition = g_WorldMatrix * vec4(in_VertexPosition * 0.004 * 8 + Offset, 1.0f);
    gl_Position = g_WorldToScreen * WSPosition;
}

#else

void main()
{
    uint VolumeID = g_VolumeID[gl_InstanceID];
    uint ParentIndex = VolumeID / 512;
    uint ChildIndex = VolumeID % 512;

    vec3 ParentOffset = IndexToOffset(ParentIndex, 16);
    vec3 ChildOffset = IndexToOffset(ChildIndex, 8);

    vec3 Offset = ParentOffset * (0.004 * 8 * 8) + ChildOffset * (0.004 * 8);

    vec4 WSPosition = g_WorldMatrix * vec4(in_VertexPosition * 0.004 * 8 + Offset, 1.0f);
    gl_Position = g_WorldToScreen * WSPosition;
}

#endif

#endif // __INCLUDE_VS_OUTLINE_GLSL__