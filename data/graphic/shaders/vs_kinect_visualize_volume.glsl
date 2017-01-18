
#ifndef __INCLUDE_VS_KINECT_VISUALIZE_VOLUME_GLSL__
#define __INCLUDE_VS_KINECT_VISUALIZE_VOLUME_GLSL__

#include "common_global.glsl"

layout (binding = 0, rg16ui) readonly uniform uimage3D vs_Volume;

layout(row_major, std140, binding = 1) uniform PerDrawCallData
{
	mat4 g_WorldMatrix;
};

out gl_PerVertex
{
    vec4 gl_Position;
};

layout(location = 0) out vec4 out_Color;

void main(void)
{
	uvec3 VoxelPosition; 
	VoxelPosition.x = gl_VertexID / (VOLUME_SIZE * VOLUME_SIZE);
	uint Index = gl_VertexID - (VoxelPosition.x * VOLUME_SIZE * VOLUME_SIZE);
	VoxelPosition.y = Index / VOLUME_SIZE;
	VoxelPosition.z = Index % VOLUME_SIZE;
    gl_Position = g_WorldToScreen * g_WorldMatrix * vec4(VoxelPosition, 1.0);
	out_Color = vec4(imageLoad(vs_Volume, ivec3(VoxelPosition)));
}

#endif // __INCLUDE_VS_KINECT_VISUALIZE_VOLUME_GLSL__