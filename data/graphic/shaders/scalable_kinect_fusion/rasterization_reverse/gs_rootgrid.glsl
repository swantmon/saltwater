
#ifndef __INCLUDE_GS_RASTERIZATION_ROOTGRID_GLSL__
#define __INCLUDE_GS_RASTERIZATION_ROOTGRID_GLSL__

// -----------------------------------------------------------------------------
// Buffer
// -----------------------------------------------------------------------------

layout(row_major, std140, binding = 0) uniform PerVolumeData
{
    ivec3 g_Offset;
    int g_Resolution;
};

layout (binding = 0, rgba16f) uniform image2D cs_VertexMap;

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
layout(points, max_vertices = 1) out;
void main()
{
    ivec2 UV;
    UV.x = in_VertexID[0] % DEPTH_IMAGE_WIDTH;
    UV.y = in_VertexID[0] / DEPTH_IMAGE_WIDTH;
    vec3 WSPosition = imageLoad(cs_VertexMap, UV).xyz;

    vec3 VSPosition = WSPosition - g_Offset * VOLUME_SIZE;
    VSPosition = (VSPosition / VOLUME_SIZE) * 2.0f - 1.0f;
    VSPosition.z = VSPosition.z * 0.5f + 0.5f;

    gl_Layer = 0;

    gl_Position = vec4(VSPosition, 1.0f);
    EmitVertex();

    EndPrimitive();
}

#endif // __INCLUDE_GS_RASTERIZATION_ROOTGRID_GLSL__