
#ifndef __INCLUDE_VS_RASTERIZE_ROOTGRID_GLSL__
#define __INCLUDE_VS_RASTERIZE_ROOTGRID_GLSL__

#include "scalable_kinect_fusion/common_tracking.glsl"

struct SInstanceData
{
    ivec3 m_Offset;
    int m_Index;
};

layout(std430, binding = 1) buffer InstanceBuffer
{
    SInstanceData g_InstanceData[];
};

layout(location = 0) in vec3 in_VertexPosition;

layout(location = 0) out flat int out_Index;

out gl_PerVertex
{
    vec4 gl_Position;
};

// -----------------------------------------------------------------------------
// Functions
// -----------------------------------------------------------------------------

void main()
{
    SInstanceData InstanceData = g_InstanceData[gl_InstanceID];
    out_Index = InstanceData.m_Index;
    vec3 Offset = InstanceData.m_Offset;

	vec4 Vertex = (g_PoseMatrix * vec4((in_VertexPosition + Offset) * VOLUME_SIZE, 1.0f));
    
	Vertex.xy = Vertex.xy * g_Intrinsics[0].m_FocalLength / Vertex.z + g_Intrinsics[0].m_FocalPoint;
	Vertex.xy = Vertex.xy / vec2(DEPTH_IMAGE_WIDTH, DEPTH_IMAGE_HEIGHT) * 2.0f - 1.0f;
	Vertex.z = 1.0f;

    gl_Position = Vertex;
}

#endif // __INCLUDE_VS_RASTERIZE_ROOTGRID_GLSL__
