
#ifndef __INCLUDE_CS_VERTEX_MAP_GLSL__
#define __INCLUDE_CS_VERTEX_MAP_GLSL__

// -----------------------------------------------------------------------------
// Defines
// -----------------------------------------------------------------------------

layout(row_major, std140, binding = 0) uniform Intrinsics
{
	vec2 FocalPoint;
	vec2 FocalLength;
	vec2 InvFocalLength;
};

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
	
	float Depth = float(imageLoad(cs_DepthBuffer, ivec2(ImagePos)).r);
	
	vec4 Vertex;
	
	Vertex.xy = Depth * (ImagePos / vec2(ImageSize) - FocalPoint) * InvFocalLength;
	Vertex.z = Depth;
	Vertex.w = 1.0;
	
	imageStore(cs_VertexMap, ivec2(ImagePos), Vertex);
}

#endif // __INCLUDE_CS_VERTEX_MAP_GLSL__