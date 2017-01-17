
#ifndef __INCLUDE_CS_VERTEX_MAP_GLSL__
#define __INCLUDE_CS_VERTEX_MAP_GLSL__

#include "tracking_common.glsl"

// -----------------------------------------------------------------------------
// Defines
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Input from engine
// -----------------------------------------------------------------------------

layout (binding = 0, r16ui) readonly uniform uimage2D cs_DepthBuffer;
layout (binding = 1, rgba32f) writeonly uniform image2D cs_VertexMap;

// -------------------------------------------------------------------------------------
// Functions
// -------------------------------------------------------------------------------------

layout (local_size_x = TILE_SIZE, local_size_y = TILE_SIZE, local_size_z = 1) in;
void main()
{
	const ivec2 ImageSize = imageSize(cs_DepthBuffer);
	
	const vec2 ImagePos = vec2(gl_GlobalInvocationID.xy);
	
	const int Depth = int(imageLoad(cs_DepthBuffer, ivec2(ImagePos)).x);
	
	vec4 Vertex;
	
	Vertex.xy = float(Depth) * (ImagePos / vec2(ImageSize) - g_FocalPoint) * g_InvFocalLength;
	Vertex.z = float(Depth);
	Vertex.w = 1.0;
	
	imageStore(cs_VertexMap, ivec2(ImagePos), Depth > 0 ? Vertex : vec4(0.0));
}

#endif // __INCLUDE_CS_VERTEX_MAP_GLSL__