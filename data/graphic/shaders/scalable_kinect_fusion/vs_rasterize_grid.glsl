
#ifndef __INCLUDE_VS_RASTERIZE_GRID_GLSL__
#define __INCLUDE_VS_RASTERIZE_GRID_GLSL__

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
    out_Index = gl_InstanceID;

    vec3 GridOffset = IndextoOffset(gl_InstanceID, g_Resolution);

    vec4 Vertex = vec4(g_Offset * g_ParentSize, 1.0f);
    Vertex.xyz += (GridOffset + in_VertexPosition) * g_CubeSize;
	Vertex = (g_InvPoseMatrix * Vertex);
    
	Vertex.xy = Vertex.xy * g_Intrinsics[0].m_FocalLength / Vertex.z + g_Intrinsics[0].m_FocalPoint;
	Vertex.xy = Vertex.xy / vec2(DEPTH_IMAGE_WIDTH, DEPTH_IMAGE_HEIGHT) * 2.0f - 1.0f;
    Vertex.z = ((Vertex.z - 0.5f) / 7.5f) * 2.0f - 1.0f;

    vec3 AABBPosition = g_Offset * g_ParentSize;
    AABBPosition += IndextoOffset(int(gl_InstanceID), g_Resolution) * g_CubeSize;
    
    out_AABBMin = AABBPosition;
    out_AABBMax = AABBPosition + g_CubeSize;

    gl_Position = Vertex;
}

#endif // __INCLUDE_VS_RASTERIZE_GRID_GLSL__
