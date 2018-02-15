
#ifndef __INCLUDE_CS_FILL_INDIRECT_GLSL__
#define __INCLUDE_CS_FILL_INDIRECT_GLSL__

#include "slam/scalable_kinect_fusion/common_indirect.glsl"

// -----------------------------------------------------------------------------
// Buffers
// -----------------------------------------------------------------------------

layout(std430, binding = 0) buffer Indirect1
{
    SIndirectBuffers g_Indirect1;
};


layout(std430, binding = 1) buffer Indirect2
{
    SIndirectBuffers g_Indirect2;
};

// -----------------------------------------------------------------------------
// Compute Shader
// -----------------------------------------------------------------------------

layout (local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main()
{
    uint InstanceCount = g_Indirect1.m_Indexed.m_InstanceCount;
    g_Indirect1.m_Draw.m_InstanceCount = InstanceCount;

    g_Indirect1.m_ComputeDiv.m_WorkGroupsX = (InstanceCount + TILE_SIZE1D - 1) / TILE_SIZE1D;
    g_Indirect1.m_ComputeDiv.m_WorkGroupsY = 1;
    g_Indirect1.m_ComputeDiv.m_WorkGroupsZ = 1;
	
	g_Indirect1.m_Compute.m_WorkGroupsX = InstanceCount;
    g_Indirect1.m_Compute.m_WorkGroupsY = 1;
    g_Indirect1.m_Compute.m_WorkGroupsZ = 1;

    InstanceCount = g_Indirect2.m_Indexed.m_InstanceCount;
    g_Indirect2.m_Draw.m_InstanceCount = InstanceCount;

    g_Indirect2.m_ComputeDiv.m_WorkGroupsX = (InstanceCount + TILE_SIZE1D - 1) / TILE_SIZE1D;
    g_Indirect2.m_ComputeDiv.m_WorkGroupsY = 1;
    g_Indirect2.m_ComputeDiv.m_WorkGroupsZ = 1;
	
	g_Indirect2.m_Compute.m_WorkGroupsX = InstanceCount;
    g_Indirect2.m_Compute.m_WorkGroupsY = 1;
    g_Indirect2.m_Compute.m_WorkGroupsZ = 1;
}

#endif // __INCLUDE_CS_FILL_INDIRECT_GLSL__