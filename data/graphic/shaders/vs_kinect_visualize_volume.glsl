
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
	VoxelPosition.x = gl_VertexID / (VOLUME_RESOLUTION * VOLUME_RESOLUTION);
	uint Index = gl_VertexID - (VoxelPosition.x * VOLUME_RESOLUTION * VOLUME_RESOLUTION);
	VoxelPosition.y = Index / VOLUME_RESOLUTION;
	VoxelPosition.z = Index % VOLUME_RESOLUTION;
    gl_Position = g_WorldToScreen * g_WorldMatrix * vec4(VoxelPosition, 1.0f);
	out_Color = vec4(imageLoad(vs_Volume, ivec3(VoxelPosition)).xy / vec2(32767.0f, 1.0f), 0.0f, 0.0f);

    ivec3 IsOnEdge = ivec3(
        VoxelPosition.x == 0 || VoxelPosition.x == VOLUME_RESOLUTION - 1,
        VoxelPosition.y == 0 || VoxelPosition.y == VOLUME_RESOLUTION - 1,
        VoxelPosition.z == 0 || VoxelPosition.z == VOLUME_RESOLUTION - 1);
    
    if (IsOnEdge.x + IsOnEdge.y + IsOnEdge.z > 1)
    {
        out_Color = vec4(0.0f, 0.0f, 1.0f, 1.0f);
    }
}

#endif // __INCLUDE_VS_KINECT_VISUALIZE_VOLUME_GLSL__