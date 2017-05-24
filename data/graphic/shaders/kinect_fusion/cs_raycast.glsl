
#ifndef __INCLUDE_CS_KINECT_INTEGRATE_VOLUME_GLSL__
#define __INCLUDE_CS_KINECT_INTEGRATE_VOLUME_GLSL__

#include "common_tracking.glsl"
#include "common_raycast.glsl"

// -----------------------------------------------------------------------------
// Constants
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Input from engine
// -----------------------------------------------------------------------------

layout(binding = 0) uniform sampler3D cs_Volume;
layout(binding = 1, MAP_TEXTURE_FORMAT) writeonly uniform image2D cs_Vertex;
layout(binding = 2, MAP_TEXTURE_FORMAT) writeonly uniform image2D cs_Normal;
layout(binding = 3, r16ui) writeonly uniform uimage2D cs_Depth;

// -------------------------------------------------------------------------------------
// Functions
// -------------------------------------------------------------------------------------

layout(local_size_x = TILE_SIZE2D, local_size_y = TILE_SIZE2D, local_size_z = 1) in;
void main()
{
	const vec2 ImageSize = imageSize(cs_DepthBuffer);

	const int u = int(gl_GlobalInvocationID.x);
	const int v = int(gl_GlobalInvocationID.y);

	const int PyramidLevel = int(log2(DEPTH_IMAGE_WIDTH / ImageSize.x));

	const vec2 Gradient = ComputeGradient(ivec2(u, v));
	//const vec2 Gradient = vec2(0.5f);

	const int Depth = int(imageLoad(cs_DepthBuffer, ivec2(u, v)).x);
	vec3 Normal = vec3(0.0f);

	if (Depth != 0)
	{
		const float z = Depth / 1000.0f;
		//const float z = 0.5;

		const vec2 FocalPoint = g_Intrinisics[PyramidLevel].m_FocalPoint;
		const vec2 InvFocalLength = g_Intrinisics[PyramidLevel].m_InvFocalLength;

		Normal.xy = Gradient / (z * InvFocalLength);
		Normal.z = -1.0f - (Gradient.x  * (u - FocalPoint.x) - Gradient.y * (v - FocalPoint.y)) / z;

		Normal = mat3(g_PoseMatrix) * Normal;
	}

	//imageStore(cs_NormalBuffer, ivec2(u, v), vec4(Gradient, 0.0f, 1.0f));
	//imageStore(cs_NormalBuffer, ivec2(u, v), vec4(Normal, 1.0f));
	imageStore(cs_NormalBuffer, ivec2(u, v), vec4(normalize(Normal), 1.0f));
}

#endif // __INCLUDE_CS_KINECT_INTEGRATE_VOLUME_GLSL__