
#ifndef __INCLUDE_CS_KINECT_INTEGRATE_VOLUME_GLSL__
#define __INCLUDE_CS_KINECT_INTEGRATE_VOLUME_GLSL__

#include "tracking_common.glsl"

// -----------------------------------------------------------------------------
// Constants
// -----------------------------------------------------------------------------

#define MU 0.1
#define MAX_WEIGHT 100.0

layout(row_major, std140, binding = 1) uniform PerDrawCallData
{
	mat4 g_WorldMatrix;
};

// -----------------------------------------------------------------------------
// Input from engine
// -----------------------------------------------------------------------------

layout (binding = 0, rg16ui) uniform uimage3D cs_Volume;
layout (binding = 1, r16ui) readonly uniform uimage2D cs_Depth;
layout (binding = 2, rgba32f) readonly uniform image2D cs_Vertex;

// -------------------------------------------------------------------------------------
// Functions
// -------------------------------------------------------------------------------------

float sq(float value)
{
	return value * value;
}

layout (local_size_x = TILE_SIZE2D, local_size_y = TILE_SIZE2D, local_size_z = 1) in;
void main()
{
	ivec2 ImageSize = imageSize(cs_Vertex);
	ivec3 PixelPosition = ivec3(gl_GlobalInvocationID.xy, 0);
	
	vec3 VolumeValue = imageLoad(cs_Vertex, PixelPosition.xy).xyz;
	VolumeValue += vec3(0.5);
	VolumeValue *= VOXEL_SIZE;
	
    vec3 Position = mat3(g_InvPoseMatrix) * VolumeValue;
    vec3 CameraX = mat3(g_KMatrix) * Position;
    vec3 Delta = mat3(g_InvPoseMatrix) * vec3(0.0, 0.0, VOXEL_SIZE);
    vec3 CameraDelta = mat3(g_KMatrix) * Delta;
	
	imageStore(cs_Volume, ivec3(gl_GlobalInvocationID), uvec4(0));
	
    for(PixelPosition.z = 0; PixelPosition.z < VOLUME_RESOLUTION; ++ PixelPosition.z, Position += Delta, CameraX += CameraDelta)
	{		
		if(Position.z < 0.0001)
        {
			continue;
		}
        vec2 Pixel = vec2(CameraX.x / CameraX.z + 0.5, CameraX.y / CameraX.z + 0.5);
        if(Pixel.x < 0.0 || Pixel.x > ImageSize.x || Pixel.y < 0.0 || Pixel.y > ImageSize.y)
        {
			continue;
		}
        ivec2 Px = ivec2(Pixel.x, Pixel.y);
        uint Depth = imageLoad(cs_Depth, Px).x;
		if (Depth == 0)
		{
			continue;
		}
        const float Diff = (Depth - CameraX.z) * sqrt(1.0 + sq(Position.x / Position.z) + sq(Position.y / Position.z));
        if(Diff > - MU)
		{
            float SDF = min(1.0, Diff / MU);
            vec2 Data = vec2(imageLoad(cs_Volume, PixelPosition).xy);
            Data.x = clamp((Data.y * Data.x + SDF)/(Data.y + 1.0), -1.0, 1.0);
            Data.y = min(Data.y + 1.0, MAX_WEIGHT);
			imageStore(cs_Volume, PixelPosition, uvec4(Data, 0, 0));
        }
    }
}

#endif // __INCLUDE_CS_KINECT_INTEGRATE_VOLUME_GLSL__