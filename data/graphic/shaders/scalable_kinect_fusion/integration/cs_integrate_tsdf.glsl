
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

#ifdef CAPTURE_COLOR
layout(binding = 1, rgba8) readonly uniform image2D cs_Color;
#endif
// -------------------------------------------------------------------------------------
// Functions
// -------------------------------------------------------------------------------------

layout (local_size_x = 8, local_size_y = 8, local_size_z = 1) in;
void main()
{    
    const vec3 CameraPosition = g_PoseMatrix[3].xyz;
    const vec3 VolumeOffset = g_RootVolumePool[g_CurrentVolumeIndex].m_Offset * VOLUME_SIZE;    
    
    ivec3 Level1Offset = ivec3(IndexToOffset(g_VolumeID[gl_WorkGroupID.x], 16 * 8));
    vec3 ParentOffset = Level1Offset * VOXEL_SIZE * 8.0f + VolumeOffset;

    ivec3 VoxelRootOffset = Level1Offset / 8;
    ivec3 VoxelLevel1InnerOffset = Level1Offset % 8;
    
    int RootGridBufferOffset = g_CurrentVolumeIndex * VOXELS_PER_ROOTGRID;
    RootGridBufferOffset += OffsetToIndex(VoxelRootOffset, 16);
    
    int Level1GridBufferOffset = g_RootGridPool[RootGridBufferOffset].m_PoolIndex * VOXELS_PER_LEVEL1GRID;
    Level1GridBufferOffset += OffsetToIndex(VoxelLevel1InnerOffset, 8);

    int Level2GridBufferOffset = g_Level1GridPool[Level1GridBufferOffset].m_PoolIndex * VOXELS_PER_LEVEL2GRID;

    int MaxWeight = 0;
    for (int i = 0; i < 8; ++ i)
    {
        vec3 WSVoxelPosition = ParentOffset + vec3(gl_LocalInvocationID.xy, i) * VOXEL_SIZE;
		
        vec3 VSVoxelPosition = (g_InvPoseMatrix * vec4(WSVoxelPosition, 1.0f)).xyz;

        vec2 CSVoxelPosition = VSVoxelPosition.xy * g_Intrinsics[0].m_FocalLength / VSVoxelPosition.z + g_Intrinsics[0].m_FocalPoint;

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
                    const float TSDF = min(SDF / TRUNCATED_DISTANCE, 1.0f);

                    int TSDFIndex = Level2GridBufferOffset + OffsetToIndex(vec3(gl_LocalInvocationID.xy, i), 8);

                    uint TSDFPoolValue = g_TSDFPool[TSDFIndex];
                    
                #ifdef CAPTURE_COLOR

                    vec2 Voxel = UnpackVoxel(TSDFPoolValue);

                    Voxel.x = (Voxel.x * Voxel.y + TSDF) / (Voxel.y + 1.0f);
                    Voxel.y = min(MAX_INTEGRATION_WEIGHT, Voxel.y + 1.0f);

                    vec3 Color = imageLoad(cs_Color, DepthCoords).rgb;
                    TSDFPoolValue = PackVoxel(Voxel.x, Voxel.y, Color);

                #else

                    vec2 Voxel = UnpackVoxel(TSDFPoolValue);

                    Voxel.x = (Voxel.x * Voxel.y + TSDF) / (Voxel.y + 1.0f);
                    Voxel.y = min(MAX_INTEGRATION_WEIGHT, Voxel.y + 1.0f);

                    TSDFPoolValue = PackVoxel(Voxel.x, Voxel.y);

                #endif // CAPTURE_COLOR
                    
                    g_TSDFPool[TSDFIndex] = TSDFPoolValue;

                    MaxWeight = max(MaxWeight, int(Voxel.y));
                }
            }
        }
    }
    atomicMax(g_Level1GridPool[Level1GridBufferOffset].m_Weight, MaxWeight);
}

#endif // __INCLUDE_CS_KINECT_INTEGRATE_TSDF_GLSL__