
#ifndef __INCLUDE_VS_RASTERIZE_GRID_GLSL__
#define __INCLUDE_VS_RASTERIZE_GRID_GLSL__

#include "scalable_kinect_fusion/common_tracking.glsl"

uvec3 Indexto3D(uint Index, uint Resolution)
{
    uint z = Index / (Resolution * Resolution);
    Index -= (z * Resolution * Resolution);
    uint y = Index / Resolution;
    uint x = Index % Resolution;
    return uvec3(x, y, z);
}

// -----------------------------------------------------------------------------
// Uniform buffers
// -----------------------------------------------------------------------------

layout(row_major, std140, binding = 2) uniform UBOTransform
{
    uint g_Resolution;
    float g_CubeSize;
    ivec3 g_Offset;
};

// -----------------------------------------------------------------------------
// Inputs
// -----------------------------------------------------------------------------

layout(location = 0) in vec3 in_VertexPosition;

// -----------------------------------------------------------------------------
// Outputs
// -----------------------------------------------------------------------------

layout(location = 0) out flat uint out_Index;

out gl_PerVertex
{
    vec4 gl_Position;
};

// -----------------------------------------------------------------------------
// Vertex shader
// -----------------------------------------------------------------------------

void main()
{
    uint Index = gl_InstanceID;
    out_Index = Index;

    uvec3 GridOffset = Indexto3D(Index, g_Resolution);

    vec4 Vertex = vec4((in_VertexPosition + g_Offset) * g_CubeSize, 1.0f);
    Vertex.xyz += GridOffset * g_CubeSize;
	Vertex = (g_PoseMatrix * Vertex);
    
	Vertex.xy = Vertex.xy * g_Intrinsics[0].m_FocalLength / Vertex.z + g_Intrinsics[0].m_FocalPoint;
	Vertex.xy = Vertex.xy / vec2(DEPTH_IMAGE_WIDTH, DEPTH_IMAGE_HEIGHT) * 2.0f - 1.0f;

    gl_Position = Vertex;
}

#endif // __INCLUDE_VS_RASTERIZE_GRID_GLSL__
