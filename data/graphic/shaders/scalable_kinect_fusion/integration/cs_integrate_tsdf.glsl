
#ifndef __INCLUDE_CS_KINECT_INTEGRATE_TSDF_GLSL__
#define __INCLUDE_CS_KINECT_INTEGRATE_TSDF_GLSL__

#include "scalable_kinect_fusion/common_scalable.glsl"
#include "scalable_kinect_fusion/common_indirect.glsl"

// -----------------------------------------------------------------------------
// Constants
// -----------------------------------------------------------------------------

#include "scalable_kinect_fusion/common_indirect.glsl"
#include "scalable_kinect_fusion/common_scalable.glsl"
#include "scalable_kinect_fusion/common_tracking.glsl"

layout(std430, binding = 6) buffer Level2Queue
{
    uint g_VolumeID[];
};

layout(std430, binding = 7) buffer Indirect
{
    SIndirectBuffers g_Indirect;
};

layout(binding = 0, r16ui) readonly uniform uimage2D cs_Depth; 

// -------------------------------------------------------------------------------------
// Functions
// -------------------------------------------------------------------------------------

layout (local_size_x = 8, local_size_y = 8, local_size_z = 1) in;
void main()
{    
    const vec3 CameraPosition = g_PoseMatrix[3].xyz;
    const vec3 VolumeOffset = g_RootVolumePool[g_CurrentVolumeIndex].m_Offset;    
    
    ivec3 VoxelCoords = ivec3(IndexToOffset(g_VolumeID[gl_GlobalInvocationID.x], 16 * 8));

    for (int i = 0; i < 8; ++ i)
    {
        ++ VoxelCoords.z;
        
        vec3 WSVoxelPosition = (VoxelCoords + vec3(0.5f, 0.5f, 0.0f)) * VOXEL_SIZE;
		WSVoxelPosition += VolumeOffset;
		
        vec3 VSVoxelPosition = (g_InvPoseMatrix * vec4(WSVoxelPosition, 1.0f)).xyz;

        vec2 CSVoxelPosition = VSVoxelPosition.xy * g_Intrinsics[0].m_FocalLength / VSVoxelPosition.z + g_Intrinsics[0].m_FocalPoint;
        //CSVoxelPosition.xy += vec2(0.5f);

        if (CSVoxelPosition.x > 0 && CSVoxelPosition.x < DEPTH_IMAGE_WIDTH && CSVoxelPosition.y > 0 && CSVoxelPosition.y < DEPTH_IMAGE_HEIGHT && VSVoxelPosition.z > 0.0f)
        {
			const ivec2 DepthCoords = ivec2(DEPTH_IMAGE_WIDTH - CSVoxelPosition.x, CSVoxelPosition.y);
            const int Depth = int(imageLoad(cs_Depth, DepthCoords).x);
            
            if (Depth != 0)
            {
                const vec2 LambdaPoint = (CSVoxelPosition.xy - g_Intrinsics[0].m_FocalPoint) * g_Intrinsics[0].m_InvFocalLength;
                const float Lambda = length(vec3(LambdaPoint, 1.0f));

                const float SDF = Depth - 1000.0f * length(CameraPosition - WSVoxelPosition) / Lambda;
                
                if (SDF >= -TRUNCATED_DISTANCE)
                {
                    /*const float TSDF = min(SDF / TRUNCATED_DISTANCE, 1.0f);

                    vec2 Voxel = imageLoad(cs_TSDFVolume, VoxelCoords).xy;

                    Voxel.x = (Voxel.x * Voxel.y + TSDF) / (Voxel.y + 1.0f);
                    Voxel.y = min(MAX_INTEGRATION_WEIGHT, Voxel.y + 1.0f);

                    imageStore(cs_TSDFVolume, VoxelCoords, vec4(Voxel, 0.0f, 0.0f));*/
                }
            }
        }
    }
}

#endif // __INCLUDE_CS_KINECT_INTEGRATE_TSDF_GLSL__