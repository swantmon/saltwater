
#ifndef __INCLUDE_FS_RASTERIZATION_ROOTGRID_GLSL__
#define __INCLUDE_FS_RASTERIZATION_ROOTGRID_GLSL__

// -----------------------------------------------------------------------------
// Buffer
// -----------------------------------------------------------------------------

layout(row_major, std140, binding = 3) uniform PerVolumeData
{
    ivec3 g_Offset;
    int g_Resolution;
};

// -----------------------------------------------------------------------------
// Inputs
// -----------------------------------------------------------------------------

layout(location = 0) in flat vec3 in_WSPosition;

layout(location = 0) out vec4 out_Color;

// -----------------------------------------------------------------------------
// Fragment shader
// -----------------------------------------------------------------------------

layout(pixel_center_integer) in vec4 gl_FragCoord;

void main()
{
    vec3 AABBMin = vec3(gl_FragCoord.xy, gl_Layer);
    AABBMin /= 16.0f;
    AABBMin += g_Offset;
    vec3 AABBMax = AABBMin + VOLUME_SIZE / 16.0f;

    AABBMin -= TRUNCATED_DISTANCE / 1000.0f;
    AABBMax += TRUNCATED_DISTANCE / 1000.0f;

    if (in_WSPosition.z > AABBMin.z && in_WSPosition.z < AABBMax.z)
    {
        if (in_WSPosition.y > AABBMin.y && in_WSPosition.y < AABBMax.y)
        {
            if (in_WSPosition.x > AABBMin.x && in_WSPosition.x < AABBMax.x)
            {
                out_Color = vec4(1.0f);
                return;
            }
        }
    }
    discard;
}

#endif // __INCLUDE_FS_RASTERIZATION_ROOTGRID_GLSL__