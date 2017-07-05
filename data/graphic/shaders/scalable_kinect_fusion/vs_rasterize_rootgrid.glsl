
#ifndef __INCLUDE_VS_RASTERIZE_ROOTGRID_GLSL__
#define __INCLUDE_VS_RASTERIZE_ROOTGRID_GLSL__

#include "scalable_kinect_fusion/common_tracking.glsl"

layout(row_major, std140, binding = 2) uniform UBOOffset
{
    vec3 g_Offset;
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
	vec4 Vertex = (g_PoseMatrix * vec4(in_VertexPosition + g_Offset, 1.0f));
	Vertex.xy = Vertex.xy * g_Intrinisics[0].m_FocalLength / Vertex.z + g_Intrinisics[0].m_FocalPoint;
	Vertex.xy /= vec2(512, 424) - vec2(512, 424) / 2;
	Vertex.z /= 8.0f;
    gl_Position = Vertex;
}

#endif // __INCLUDE_VS_RASTERIZE_ROOTGRID_GLSL__
