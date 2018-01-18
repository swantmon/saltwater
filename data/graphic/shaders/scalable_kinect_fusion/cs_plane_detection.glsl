
#ifndef __INCLUDE_CS_PLANE_DETECTION_GLSL__
#define __INCLUDE_CS_PLANE_DETECTION_GLSL__

layout(row_major, std140, binding = 0) uniform HistogramSizes
{
    ivec4 g_HistogramSizes;
};

layout(std430, binding = 0) buffer Histogram
{
    int g_Histogram[];
};

// -----------------------------------------------------------------------------
// Input from engine
// -----------------------------------------------------------------------------

layout (binding = 0, MAP_TEXTURE_FORMAT) uniform image2D cs_VertexMap;
layout (binding = 1, MAP_TEXTURE_FORMAT) uniform image2D cs_NormalMap;

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
	
	vec3 Normal = normalize(cross(Vertex1 - Vertex0, Vertex2 - Vertex0));

	float Inclination = acos(Normal.z);
	float Azimuth = atan(Normal.y, Normal.x);
	
	g_Histogram[gl_LocalInvocationIndex] = int(gl_LocalInvocationIndex);
}

#endif // __INCLUDE_CS_PLANE_DETECTION_GLSL__