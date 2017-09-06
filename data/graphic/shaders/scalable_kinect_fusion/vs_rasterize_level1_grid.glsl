
#ifndef __INCLUDE_VS_RASTERIZE_LEVEL1_GRID_GLSL__
#define __INCLUDE_VS_RASTERIZE_LEVEL1_GRID_GLSL__

#include "scalable_kinect_fusion/common_scalable.glsl"
#include "scalable_kinect_fusion/common_tracking.glsl"

// -----------------------------------------------------------------------------
// Uniform buffers
// -----------------------------------------------------------------------------

layout(row_major, std140, binding = 2) uniform UBOTransform
{
    ivec3 g_Offset;
    int g_Resolution;
    float g_CubeSize;
    float g_ParentSize;
};

layout(std430, binding = 2) buffer Level1Queue
{
    uint g_VolumeID[];
};

// -----------------------------------------------------------------------------
// Inputs
// -----------------------------------------------------------------------------

layout(location = 0) in vec3 in_VertexPosition;

// -----------------------------------------------------------------------------
// Outputs
// -----------------------------------------------------------------------------

layout(location = 0) out uint out_Index;
layout(location = 1) out flat vec3 out_AABBMin;
layout(location = 2) out flat vec3 out_AABBMax;

out gl_PerVertex
{
    vec4 gl_Position;
};

// -----------------------------------------------------------------------------
// Vertex shader
// -----------------------------------------------------------------------------

void main()
{
    uint ParentIndex = g_VolumeID[gl_InstanceID];
    uint GridIndex = gl_VertexID / 8;

    uint Level1Index = ParentIndex * 512 + GridIndex;

    vec3 ParentOffset = IndexToOffset(ParentIndex, 16) * g_ParentSize;

    vec4 Vertex = vec4(g_Offset * VOLUME_SIZE, 1.0f);
    Vertex.xyz += (in_VertexPosition * g_CubeSize) + ParentOffset;

	Vertex = (g_InvPoseMatrix * Vertex);
	Vertex.xy = Vertex.xy * g_Intrinsics[0].m_FocalLength / Vertex.z + g_Intrinsics[0].m_FocalPoint;
	Vertex.xy = Vertex.xy / vec2(DEPTH_IMAGE_WIDTH, DEPTH_IMAGE_HEIGHT) * 2.0f - 1.0f;
    Vertex.z = 1.0f;//Vertex.z / (8.0f + VOLUME_SIZE);

    vec3 AABBPosition = g_Offset * VOLUME_SIZE + ParentOffset;
    
    out_AABBMin = AABBPosition;
    out_AABBMax = AABBPosition + g_CubeSize;
    out_Index = Level1Index;
    gl_Position = Vertex;
}

#endif // __INCLUDE_VS_RASTERIZE_LEVEL1_GRID_GLSL__
