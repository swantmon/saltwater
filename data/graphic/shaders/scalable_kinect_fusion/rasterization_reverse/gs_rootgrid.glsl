
#ifndef __INCLUDE_GS_RASTERIZATION_ROOTGRID_GLSL__
#define __INCLUDE_GS_RASTERIZATION_ROOTGRID_GLSL__

#include "scalable_kinect_fusion/common_tracking.glsl"

// -----------------------------------------------------------------------------
// Buffer
// -----------------------------------------------------------------------------

layout(row_major, std140, binding = 3) uniform PerVolumeData
{
    ivec3 g_Offset;
    int g_Resolution;
};

layout(binding = 0, rgba16f) uniform image2D cs_VertexMap;

// -----------------------------------------------------------------------------
// Inputs
// -----------------------------------------------------------------------------

in gl_PerVertex
{
    vec4 gl_Position;
} gl_in[];

layout(location = 0) in flat int in_VertexID[];

// -----------------------------------------------------------------------------
// Outputs
// -----------------------------------------------------------------------------

out gl_PerVertex
{
    vec4 gl_Position;
};

out int gl_Layer;

// -----------------------------------------------------------------------------
// Geometry shader
// -----------------------------------------------------------------------------

layout(points) in;
layout(line_strip, max_vertices = 2) out;
void main()
{
    ivec2 UV;
    UV.x = in_VertexID[0] % DEPTH_IMAGE_WIDTH;
    UV.y = in_VertexID[0] / DEPTH_IMAGE_WIDTH;
    vec3 WSPosition = imageLoad(cs_VertexMap, UV).xyz;
    WSPosition = (g_PoseMatrix * vec4(WSPosition, 1.0f)).xyz;

    vec3 CameraDirection = normalize(WSPosition - g_PoseMatrix[3].xyz);

    vec3 WSLinePositions[2];
    WSLinePositions[0] = WSPosition - CameraDirection * TRUNCATED_DISTANCE / 1000.0f;
    WSLinePositions[1] = WSPosition + CameraDirection * TRUNCATED_DISTANCE / 1000.0f;

    vec3 VSLinePositions[2];
    for(int i = 0; i < 2; ++ i)
    {
        VSLinePositions[i] = WSLinePositions[i] - g_Offset * VOLUME_SIZE;
        VSLinePositions[i] = (VSLinePositions[i] / VOLUME_SIZE) * 2.0f - 1.0f;
        VSLinePositions[i].z = VSLinePositions[i].z * 0.5f + 0.5f;
    }

    int Layers[2];
    for(int i = 0; i < 2; ++ i)
    {
        Layers[i] = int(VSLinePositions[i].z * 16);
    }

    int MinLayer = min(Layers[0], Layers[1]);
    int MaxLayer = max(Layers[0], Layers[1]);

    for(int LayerIndex = MinLayer; LayerIndex <= MaxLayer; ++ LayerIndex)
    {
        for(int i = 0; i < 2; ++ i)
        {
            gl_Layer = LayerIndex;
            gl_Position = vec4(VSLinePositions[i], 1.0f);

            EmitVertex();
        }
        EndPrimitive();
    }
}

#endif // __INCLUDE_GS_RASTERIZATION_ROOTGRID_GLSL__