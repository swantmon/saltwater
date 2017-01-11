
#ifndef __INCLUDE_CS_NORMAL_MAP_GLSL__
#define __INCLUDE_CS_NORMAL_MAP_GLSL__

// -----------------------------------------------------------------------------
// Defines
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Input from engine
// -----------------------------------------------------------------------------

layout (binding = 0, rgba32f) readonly uniform image2D cs_VertexMap;
layout (binding = 1, rgba32f) writeonly uniform image2D cs_NormalMap;

// -------------------------------------------------------------------------------------
// Functions
// -------------------------------------------------------------------------------------
layout (local_size_x = TILE_SIZE, local_size_y = TILE_SIZE, local_size_z = 1) in;
void main()
{
	const ivec2 ImageSize = imageSize(cs_VertexMap);
	
	const int x = int(gl_GlobalInvocationID.x);
	const int y = int(gl_GlobalInvocationID.y);
	
	vec3 Vertex0 = imageLoad(cs_VertexMap, ivec2(x, y)).xyz;
	vec3 Vertex1 = imageLoad(cs_VertexMap, ivec2(x + 1, y)).xyz;
	vec3 Vertex2 = imageLoad(cs_VertexMap, ivec2(x, y + 1)).xyz;
	
	vec3 Normal = cross(Vertex1 - Vertex0, Vertex2 - Vertex0);
	Normal = normalize(Normal);
	
	imageStore(cs_NormalMap, ivec2(x, y), vec4(Normal, 1.0));
}

#endif // __INCLUDE_CS_NORMAL_MAP_GLSL__