
#ifndef __INCLUDE_CS_KINECT_INTEGRATE_GLSL__
#define __INCLUDE_CS_KINECT_INTEGRATE_GLSL__

#include "scalable_kinect_fusion/common_tracking.glsl"

// -----------------------------------------------------------------------------
// Constants
// -----------------------------------------------------------------------------

layout(row_major, std140, binding = 2) uniform UBOOffset
{
    vec3 g_Offset;
};

// -----------------------------------------------------------------------------
// Input from engine
// -----------------------------------------------------------------------------

layout(binding = 0, rg16f) uniform image3D cs_TSDFVolume;
layout(binding = 1, r16ui) readonly uniform uimage2D cs_Depth;

#ifdef CAPTURE_COLOR
layout(binding = 2, rgba8) uniform image3D cs_ColorVolume;
layout(binding = 3, rgba8) readonly uniform image2D cs_Color;
#endif

// -------------------------------------------------------------------------------------
// Functions
// -------------------------------------------------------------------------------------

layout (local_size_x = TILE_SIZE2D, local_size_y = TILE_SIZE2D, local_size_z = 1) in;
void main()
{
    const int x = int(gl_GlobalInvocationID.x);
    const int y = int(gl_GlobalInvocationID.y);
    
    const vec3 CameraPosition = g_PoseMatrix[3].xyz;
    ivec3 VoxelCoords = ivec3(x, y, 0);

    for (VoxelCoords.z = 0; VoxelCoords.z < VOLUME_RESOLUTION; ++ VoxelCoords.z)
    {
        vec3 WSVoxelPosition = (VoxelCoords + vec3(0.5f, 0.5f, 0.0f)) * VOXEL_SIZE;
		WSVoxelPosition += g_Offset;
		
        vec3 VSVoxelPosition = (g_InvPoseMatrix * vec4(WSVoxelPosition, 1.0f)).xyz;

        vec2 CSVoxelPosition = VSVoxelPosition.xy * g_Intrinisics[0].m_FocalLength / VSVoxelPosition.z + g_Intrinisics[0].m_FocalPoint;
        //CSVoxelPosition.xy += vec2(0.5f);

        if (CSVoxelPosition.x > 0 && CSVoxelPosition.x < DEPTH_IMAGE_WIDTH && CSVoxelPosition.y > 0 && CSVoxelPosition.y < DEPTH_IMAGE_HEIGHT && VSVoxelPosition.z > 0.0f)
        {
			const ivec2 DepthCoords = ivec2(DEPTH_IMAGE_WIDTH - CSVoxelPosition.x, CSVoxelPosition.y);
            const int Depth = int(imageLoad(cs_Depth, DepthCoords).x);
            
            if (Depth != 0)
            {
                const vec2 LambdaPoint = (CSVoxelPosition.xy - g_Intrinisics[0].m_FocalPoint) * g_Intrinisics[0].m_InvFocalLength;
                const float Lambda = length(vec3(LambdaPoint, 1.0f));

                const float SDF = Depth - 1000.0f * length(CameraPosition - WSVoxelPosition) / Lambda;
                
                if (SDF >= -TRUNCATED_DISTANCE)
                {
                    const float TSDF = min(SDF / TRUNCATED_DISTANCE, 1.0f);

                    vec2 Voxel = imageLoad(cs_TSDFVolume, VoxelCoords).xy;

                    Voxel.x = (Voxel.x * Voxel.y + TSDF) / (Voxel.y + 1.0f);
                    Voxel.y = min(MAX_INTEGRATION_WEIGHT, Voxel.y + 1.0f);

                    imageStore(cs_TSDFVolume, VoxelCoords, vec4(Voxel, 0.0f, 0.0f));

                    #ifdef CAPTURE_COLOR
                    const vec3 OldColor = imageLoad(cs_ColorVolume, VoxelCoords).rgb;
                    const vec3 Color = imageLoad(cs_Color, DepthCoords).rgb;
                    if (Color.r != 0.0f && Color.g != 0.0f && Color.b != 0.0f)
                    {
                        const vec3 NewColor = mix(OldColor, Color, abs(TSDF));
                        imageStore(cs_ColorVolume, VoxelCoords, vec4(NewColor, 1.0f));
                    }
                    #endif
                }
            }
        }
    }
}

#endif // __INCLUDE_CS_KINECT_INTEGRATE_GLSL__