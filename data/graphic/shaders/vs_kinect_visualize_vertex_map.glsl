
#ifndef __INCLUDE_VS_KINECT_VISUALIZE_VERTEX_MAP_GLSL__
#define __INCLUDE_VS_KINECT_VISUALIZE_VERTEX_MAP_GLSL__

#include "common_global.glsl"

layout(binding = 0, rgba32f) readonly uniform image2D vs_VertexMap;

out gl_PerVertex
{
    vec4 gl_Position;
};

layout(location = 0) out flat int IsValid;

void main(void)
{
	ivec2 ImageSize = imageSize(vs_VertexMap);
	
	ivec2 VertexMapPosition;
	VertexMapPosition.x = gl_VertexID / ImageSize.x;
	VertexMapPosition.y = gl_VertexID % ImageSize.y;
	
	vec4 Vertex = vec4(imageLoad(vs_VertexMap, VertexMapPosition).xyz * 0.01, 1.0);
	
	IsValid = Vertex.x != 0.0 ? 1 : 0;
	
	gl_Position = g_WorldToScreen * Vertex;
	//gl_Position = vec4(VertexMapPosition, 0.0, 1.0);
}

#endif // __INCLUDE_VS_KINECT_VISUALIZE_VERTEX_MAP_GLSL__