
#ifndef __INCLUDE_FS_OUTLINE_GLSL__
#define __INCLUDE_FS_OUTLINE_GLSL__

#include "../../plugins/slam/scalable/common_raycast.glsl"

layout(std140, binding = 0) uniform PerDrawCallData
{
    mat4 g_WorldMatrix;
	vec4 g_Color;
	vec3 g_Normal;
};

layout (binding = 0, rgba8) uniform image2D cs_Texture;

layout(location = 0) in vec3 in_WSPosition;

void main()
{
	mat3 SaltwaterToReconstruction = mat3(
        1.0f, 0.0f,  0.0f,
        0.0f, 0.0f, -1.0f,
        0.0f, 1.0f,  0.0f
    );
 
    mat3 ReconstructionToSaltwater = mat3(
        1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f,
        0.0f, -1.0f, 0.0f
    );

    vec3 CameraPosition = SaltwaterToReconstruction * in_WSPosition;
    vec3 RayDirection = SaltwaterToReconstruction * g_Normal;
 
    RayDirection.x = RayDirection.x == 0.0f ? 1e-15f : RayDirection.x;
    RayDirection.y = RayDirection.y == 0.0f ? 1e-15f : RayDirection.y;
    RayDirection.z = RayDirection.z == 0.0f ? 1e-15f : RayDirection.z;

    vec3 Vertex = GetPosition(CameraPosition, RayDirection);
    vec3 Color = GetColor(Vertex);

    imageStore(cs_Texture, ivec2(gl_FragCoord.xy), vec4(Color, 1.0f));
}

#endif // __INCLUDE_FS_OUTLINE_GLSL__