
#ifndef __INCLUDE_GS_RASTERIZATION_ROOTGRID_GLSL__
#define __INCLUDE_GS_RASTERIZATION_ROOTGRID_GLSL__

#include "slam/common_tracking.glsl"

// -----------------------------------------------------------------------------
// Buffer
// -----------------------------------------------------------------------------

layout(std140, binding = 3) uniform PerVolumeData
{
    ivec3 g_Offset;
    int g_BufferOffset;
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

layout(location = 0) out flat vec3 out_WSPosition;
layout(location = 1) out flat ivec2 out_Layers;

const int g_Resolution = ROOT_RESOLUTION * LEVEL1_RESOLUTION;

// -----------------------------------------------------------------------------
// Geometry shader
// -----------------------------------------------------------------------------

#ifdef CONSERVATIVE_RASTERIZATION_AVAILABLE

layout(points) in;
layout(line_strip, max_vertices = 2) out;
void main()
{
    ivec2 UV;
    UV.x = in_VertexID[0] % DEPTH_IMAGE_WIDTH;
    UV.y = in_VertexID[0] / DEPTH_IMAGE_WIDTH;
    vec3 Position = imageLoad(cs_VertexMap, UV).xyz;

    if (Position.x != 0.0f)
    {
        vec3 WSPosition = (g_PoseMatrix * vec4(Position, 1.0f)).xyz;

        vec3 AABBMin = g_Offset * VOLUME_SIZE;
        vec3 AABBMax = AABBMin + VOLUME_SIZE + TRUNCATED_DISTANCE / 1000.0f;
        AABBMin -= TRUNCATED_DISTANCE / 1000.0f;

        if (WSPosition.x > AABBMin.x && WSPosition.x < AABBMax.x &&
            WSPosition.y > AABBMin.y && WSPosition.y < AABBMax.y &&
            WSPosition.z > AABBMin.z && WSPosition.z < AABBMax.z)
        {
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
                Layers[i] = int(VSLinePositions[i].z * g_Resolution);
            }

            int MinLayer = max(               0, min(Layers[0], Layers[1]));
            int MaxLayer = min(g_Resolution - 1, max(Layers[0], Layers[1]));

            for(int i = 0; i < 2; ++ i)
            {
                gl_Position = vec4(VSLinePositions[i], 1.0f);
                out_WSPosition = WSPosition;
                out_Layers = ivec2(MinLayer, MaxLayer);

                EmitVertex();
            }
            EndPrimitive();
        }
    }
}

#else

layout(points) in;
layout(triangle_strip, max_vertices = 24) out;
void main()
{
    ivec2 UV;
    UV.x = in_VertexID[0] % DEPTH_IMAGE_WIDTH;
    UV.y = in_VertexID[0] / DEPTH_IMAGE_WIDTH;
    vec3 Position = imageLoad(cs_VertexMap, UV).xyz;

    if (Position.x != 0.0f)
    {
        vec3 WSPosition = (g_PoseMatrix * vec4(Position, 1.0f)).xyz;

        vec3 AABBMin = g_Offset * VOLUME_SIZE;
        vec3 AABBMax = AABBMin + VOLUME_SIZE + TRUNCATED_DISTANCE / 1000.0f;
        AABBMin -= TRUNCATED_DISTANCE / 1000.0f;

        if (WSPosition.x > AABBMin.x && WSPosition.x < AABBMax.x &&
            WSPosition.y > AABBMin.y && WSPosition.y < AABBMax.y &&
            WSPosition.z > AABBMin.z && WSPosition.z < AABBMax.z)
        {
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
                Layers[i] = int(VSLinePositions[i].z * g_Resolution);
            }

            int MinLayer = max(               0, min(Layers[0], Layers[1]));
            int MaxLayer = min(g_Resolution - 1, max(Layers[0], Layers[1]));

            vec2 AABBMin = VSLinePositions[0].xy;
            vec2 AABBMax = VSLinePositions[1].xy;

            if (AABBMin.x > AABBMax.x)
            {
                float Temp = AABBMin.x;
                AABBMin.x = AABBMax.x;
                AABBMax.x = Temp;
            }

            if (AABBMin.y > AABBMax.y)
            {
                float Temp = AABBMin.y;
                AABBMin.y = AABBMax.y;
                AABBMax.y = Temp;
            }

            AABBMin -= 1.0f / g_Resolution;
            AABBMax += 1.0f / g_Resolution;

            for(int LayerIndex = MinLayer; LayerIndex <= MaxLayer; ++ LayerIndex)
            {
                // Face 1
                gl_Layer = LayerIndex + g_BufferOffset * g_Resolution;
                gl_Position = vec4(AABBMin.x, AABBMin.y, 1.0f, 1.0f);
                out_WSPosition = WSPosition;
                EmitVertex();

                gl_Layer = LayerIndex + g_BufferOffset * g_Resolution;
                gl_Position = vec4(AABBMax.x, AABBMin.y, 1.0f, 1.0f);
                out_WSPosition = WSPosition;
                EmitVertex();

                gl_Layer = LayerIndex + g_BufferOffset * g_Resolution;
                gl_Position = vec4(AABBMax.x, AABBMax.y, 1.0f, 1.0f);
                out_WSPosition = WSPosition;
                EmitVertex();

                // Face 2
                gl_Layer = LayerIndex + g_BufferOffset * g_Resolution;
                gl_Position = vec4(AABBMax.x, AABBMax.y, 1.0f, 1.0f);
                out_WSPosition = WSPosition;
                EmitVertex();

                gl_Layer = LayerIndex + g_BufferOffset * g_Resolution;
                gl_Position = vec4(AABBMin.x, AABBMax.y, 1.0f, 1.0f);
                out_WSPosition = WSPosition;
                EmitVertex();

                gl_Layer = LayerIndex + g_BufferOffset * g_Resolution;
                gl_Position = vec4(AABBMin.x, AABBMin.y, 1.0f, 1.0f);
                out_WSPosition = WSPosition;
                EmitVertex();

                EndPrimitive();
            }
        }
    }
}

#endif

#endif // __INCLUDE_GS_RASTERIZATION_ROOTGRID_GLSL__