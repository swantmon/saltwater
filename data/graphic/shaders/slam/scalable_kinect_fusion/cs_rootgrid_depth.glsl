
#ifndef __INCLUDE_CS_ROOTGRID_DEPTH_GLSL__
#define __INCLUDE_CS_ROOTGRID_DEPTH_GLSL__

#include "slam/common_tracking.glsl"

// -----------------------------------------------------------------------------
// Constants
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Input from engine
// -----------------------------------------------------------------------------

layout(std140, binding = 2) uniform UBOOffset
{
    vec3 g_Offset;
};

layout(binding = 0, r16ui) readonly uniform uimage2D cs_Depth;
layout(binding = 0, offset = 0) uniform atomic_uint cs_Counter;

// -------------------------------------------------------------------------------------
// Functions
// -------------------------------------------------------------------------------------

layout (local_size_x = TILE_SIZE2D, local_size_y = TILE_SIZE2D, local_size_z = 1) in;
void main()
{
	const ivec2 ImageSize = imageSize(cs_Depth);
	
	const vec2 ImagePos = vec2(gl_GlobalInvocationID.xy);
	
    const vec2 FocalPoint = g_Intrinsics[0].m_FocalPoint;
    const vec2 InvFocalLength = g_Intrinsics[0].m_InvFocalLength;

	const float Depth = imageLoad(cs_Depth, ivec2(ImagePos)).x / 1000.0f;
		
	vec3 Vertex;	
	Vertex.xy = Depth * (ImagePos - FocalPoint) * InvFocalLength;
	Vertex.z = Depth;
	
	Vertex = (g_PoseMatrix * vec4(Vertex, 1.0)).xyz;
	
	vec3 BBMin = g_Offset;
	vec3 BBMax = BBMin + VOLUME_SIZE;
	
	if (Vertex.x < BBMax.x && Vertex.x > BBMin.x &&
	    Vertex.y < BBMax.y && Vertex.y > BBMin.y &&
	    Vertex.z < BBMax.z && Vertex.z > BBMin.z)
	{
		atomicCounterIncrement(cs_Counter);
	}
}

#endif // __INCLUDE_CS_ROOTGRID_DEPTH_GLSL__