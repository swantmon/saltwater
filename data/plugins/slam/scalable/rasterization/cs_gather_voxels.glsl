
#ifndef __INCLUDE_CS_GATHER_VOXELS_GLSL__
#define __INCLUDE_CS_GATHER_VOXELS_GLSL__

#include "../../plugins/slam/scalable/common_scalable.glsl"
#include "../../plugins/slam/scalable/common_indirect.glsl"

// -----------------------------------------------------------------------------
// Buffers
// -----------------------------------------------------------------------------

layout(binding = 1, r8ui) uniform uimage3D cs_Volume;

layout(std430, binding = 0) buffer Level1IndirectBuffer
{
    SIndirectBuffers g_Level1Indirect;
};

layout(std430, binding = 1) buffer Level2IndirectBuffer
{
    SIndirectBuffers g_Level2Indirect;
};

layout(std430, binding = 2) buffer Level1Queue
{
    uint g_Level1VolumeID[];
};

layout(std430, binding = 3) buffer Level2Queue
{
    uint g_Level2VolumeID[];
};

// -----------------------------------------------------------------------------
// Compute Shader
// -----------------------------------------------------------------------------

const int g_Resolution = ROOT_RESOLUTION * LEVEL1_RESOLUTION;

shared int TaggedSum;

layout (local_size_x = LEVEL1_RESOLUTION, local_size_y = LEVEL1_RESOLUTION, local_size_z = LEVEL1_RESOLUTION) in;
void main()
{
    uint IsTagged = imageLoad(cs_Volume, ivec3(gl_GlobalInvocationID)).x;
    imageStore(cs_Volume, ivec3(gl_GlobalInvocationID), uvec4(0));

    if (gl_LocalInvocationIndex == 0)
    {
        TaggedSum = 0;
    }

    barrier();

    if (IsTagged > 0)
    {
        uint Index = atomicAdd(g_Level2Indirect.m_Indexed.m_InstanceCount, 1);
        g_Level2VolumeID[Index] = OffsetToIndex(vec3(gl_GlobalInvocationID), ROOT_RESOLUTION * LEVEL1_RESOLUTION);
        
        if (atomicAdd(TaggedSum, 1) == 1)
        {
            uint Index = atomicAdd(g_Level1Indirect.m_Indexed.m_InstanceCount, 1);
            g_Level1VolumeID[Index] = OffsetToIndex(vec3(gl_WorkGroupID), ROOT_RESOLUTION);
        }
    }
}

#endif // __INCLUDE_CS_GATHER_VOXELS_GLSL__