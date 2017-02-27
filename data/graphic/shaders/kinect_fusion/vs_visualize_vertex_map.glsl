
#ifndef __INCLUDE_VS_KINECT_VISUALIZE_VERTEX_MAP_GLSL__
#define __INCLUDE_VS_KINECT_VISUALIZE_VERTEX_MAP_GLSL__

#include "common_global.glsl"

layout(binding = 0, rgba32f) readonly uniform image2D vs_VertexMap;

layout(row_major, std140, binding = 1) uniform PerDrawCallData
{
	mat4 g_WorldMatrix;
};

out gl_PerVertex
{
    vec4 gl_Position;
};

layout(location = 0) out flat int IsValid;
layout(location = 1) out flat ivec2 TexCoords;

void main(void)
{
	ivec2 ImageSize = imageSize(vs_VertexMap);
	
	ivec2 VertexMapPosition;
	VertexMapPosition.x = gl_VertexID / ImageSize.x;
	VertexMapPosition.y = gl_VertexID % ImageSize.x;
	
	vec4 Vertex = vec4(imageLoad(vs_VertexMap, VertexMapPosition).xyz, 1.0);
    
    IsValid = Vertex.x != 0.0 ? 1 : 0;

    TexCoords = VertexMapPosition;
    Vertex.xyz *= 1000.0f;
	gl_Position = g_WorldToScreen * g_WorldMatrix * Vertex;
}

#endif // __INCLUDE_VS_KINECT_VISUALIZE_VERTEX_MAP_GLSL__