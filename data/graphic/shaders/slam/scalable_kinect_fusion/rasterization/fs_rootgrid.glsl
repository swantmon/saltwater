
#ifndef __INCLUDE_FS_RASTERIZATION_ROOTGRID_GLSL__
#define __INCLUDE_FS_RASTERIZATION_ROOTGRID_GLSL__

// -----------------------------------------------------------------------------
// Buffer
// -----------------------------------------------------------------------------

layout(binding = 1, r8ui) uniform uimage3D cs_Volume;

layout(std140, binding = 3) uniform PerVolumeData
{
    ivec3 g_Offset;
    int g_BufferOffset;
};

// -----------------------------------------------------------------------------
// Inputs
// -----------------------------------------------------------------------------

layout(location = 0) in flat vec3 in_WSPosition;
layout(location = 1) in flat ivec2 in_Layers;

layout(location = 0) out vec4 out_Color;

const int g_Resolution = ROOT_RESOLUTION * LEVEL1_RESOLUTION;

// -----------------------------------------------------------------------------
// Fragment shader
// -----------------------------------------------------------------------------

layout(pixel_center_integer) in vec4 gl_FragCoord;

void main()
{
    for(int LayerIndex = in_Layers[0]; LayerIndex <= in_Layers[1]; ++ LayerIndex)
    {
        vec3 AABBMin = vec3(gl_FragCoord.xy, LayerIndex);
        AABBMin /= g_Resolution;
        AABBMin *= VOLUME_SIZE;
        AABBMin += g_Offset * VOLUME_SIZE;
        vec3 AABBMax = AABBMin + VOLUME_SIZE / float(g_Resolution);

        AABBMin -= TRUNCATED_DISTANCE / 1000.0f;
        AABBMax += TRUNCATED_DISTANCE / 1000.0f;

        if (in_WSPosition.z > AABBMin.z && in_WSPosition.z < AABBMax.z)
        {
            if (in_WSPosition.y > AABBMin.y && in_WSPosition.y < AABBMax.y)
            {
                if (in_WSPosition.x > AABBMin.x && in_WSPosition.x < AABBMax.x)
                {
                    imageStore(cs_Volume, ivec3(gl_FragCoord.xy, LayerIndex), ivec4(1.0f));
                }
            }
        }
    }
}

#endif // __INCLUDE_FS_RASTERIZATION_ROOTGRID_GLSL__