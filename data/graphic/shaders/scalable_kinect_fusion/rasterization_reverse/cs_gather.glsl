
#ifndef __INCLUDE_CS_GATHER_GATHER_GLSL__
#define __INCLUDE_CS_GATHER_GATHER_GLSL__

#include "scalable_kinect_fusion/common_scalable.glsl"
#include "scalable_kinect_fusion/common_indirect.glsl"

// -----------------------------------------------------------------------------
// Buffers
// -----------------------------------------------------------------------------

layout(binding = 1, r8ui) uniform uimage3D cs_Volume;

layout(std430, binding = 2) buffer VolumeQueue
{
    uint g_VolumeID[];
};

layout(std430, binding = 3) buffer IndirectBuffer
{
    SIndirectBuffers g_Indirect;
};
// -----------------------------------------------------------------------------
// Compute Shader
// -----------------------------------------------------------------------------

layout (local_size_x = 16, local_size_y = 16, local_size_z = 1) in;
void main()
{
    uint IsTagged = imageLoad(cs_Volume, ivec3(gl_GlobalInvocationID)).x;

    if (IsTagged > 1)
    {
        uint Index = atomicAdd(g_Indirect.m_Indexed.m_InstanceCount, 1);
        g_VolumeID[Index] = OffsetToIndex(gl_GlobalInvocationID, 16);

        imageStore(cs_Volume, ivec3(gl_GlobalInvocationID), uvec4(0));
    }
}

#endif // __INCLUDE_CS_GATHER_GATHER_GLSL__