
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

layout (local_size_x = TILE_SIZE2D, local_size_y = TILE_SIZE2D, local_size_z = 1) in;
void main()
{
	const ivec2 ImageSize = imageSize(cs_DepthBuffer);
	
    const float PyramidFactor = 1.0f / (DEPTH_IMAGE_WIDTH / ImageSize.x);

	const vec2 ImagePos = vec2(gl_GlobalInvocationID.xy);
	
	const float Depth = imageLoad(cs_DepthBuffer, ivec2(ImagePos)).x / 1000.0f;
	
	vec4 Vertex;
	
	Vertex.xy = Depth * (ImagePos - g_FocalPoint * PyramidFactor) * (g_InvFocalLength / PyramidFactor);
	Vertex.z = Depth;
	Vertex.w = 1.0f;

    Vertex = g_PoseMatrix * Vertex;
	
	imageStore(cs_VertexMap, ivec2(ImagePos), Depth > 0.0f ? Vertex : vec4(0.0f));
}

#endif // __INCLUDE_CS_VERTEX_MAP_GLSL__