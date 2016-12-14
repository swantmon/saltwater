
#ifndef __INCLUDE_VS_VOXEL_GLSL__
#define __INCLUDE_VS_VOXEL_GLSL__

#include "common_global.glsl"

out gl_PerVertex
{
    vec4 gl_Position;
};

layout(row_major, std140, binding = 1) uniform PerDrawCall
{
	float Scale;
};

layout(binding = 0, r16_snorm) readonly uniform image3D vs_VoxelData;

layout(location = 0) in vec3 VertexPosition;

layout(location = 0) out float out_Color;

void main(void)
{
	uvec3 VoxelPosition;
	VoxelPosition.x = gl_VertexID / (CUBE_WIDTH * CUBE_WIDTH);
	uint Index = gl_VertexID - (VoxelPosition.x * CUBE_WIDTH * CUBE_WIDTH);
	VoxelPosition.y = Index / CUBE_WIDTH;
	VoxelPosition.z = Index % CUBE_WIDTH;
	float Voxel = imageLoad(vs_VoxelData, ivec3(VoxelPosition)).r;
	out_Color = Voxel > 0.0 ? 1.0 : 0.0;
    gl_Position = g_WorldToScreen * vec4(VertexPosition * Scale, 1.0);
}

#endif // __INCLUDE_VS_VOXEL_GLSL__