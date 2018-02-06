
#ifndef __INCLUDE_CS_NORMAL_MAP_GLSL__
#define __INCLUDE_CS_NORMAL_MAP_GLSL__

// -----------------------------------------------------------------------------
// Defines
// -----------------------------------------------------------------------------

const float NaN = 0.0 / 0.0;

// -----------------------------------------------------------------------------
// Input from engine
// -----------------------------------------------------------------------------

layout (binding = 0, MAP_TEXTURE_FORMAT) readonly uniform image2D cs_VertexMap;
layout (binding = 1, MAP_TEXTURE_FORMAT) writeonly uniform image2D cs_NormalMap;

// -------------------------------------------------------------------------------------
// Functions
// -------------------------------------------------------------------------------------
layout (local_size_x = TILE_SIZE2D, local_size_y = TILE_SIZE2D, local_size_z = 1) in;
void main()
{
	const ivec2 ImageSize = imageSize(cs_VertexMap);
	
	const int x = int(gl_GlobalInvocationID.x);
	const int y = int(gl_GlobalInvocationID.y);
	
	vec3 Vertex0 = imageLoad(cs_VertexMap, ivec2(x, y)).xyz;
	vec3 Vertex1 = imageLoad(cs_VertexMap, ivec2(x + 1, y)).xyz;
	vec3 Vertex2 = imageLoad(cs_VertexMap, ivec2(x, y + 1)).xyz;
	
	vec3 Normal = cross(Vertex1 - Vertex0, Vertex2 - Vertex0);

	const vec3 One = vec3(1.0);	
	bool IsValid = dot(Vertex0, One) != 0.0 && dot(Vertex1, One) != 0.0 && dot(Vertex2, One) != 0.0;
	
	imageStore(cs_NormalMap, ivec2(x, y), IsValid ? vec4(normalize(Normal), 0.0) : vec4(-1337.0));
}

#endif // __INCLUDE_CS_NORMAL_MAP_GLSL__