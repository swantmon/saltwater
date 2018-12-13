#ifndef __INCLUDE_CS_FUSEPANO_GLSL__
#define __INCLUDE_CS_FUSEPANO_GLSL__

// #define TILE_SIZE 8
// #define PANORAMA_TYPE rgba8

// -----------------------------------------------------------------------------
// Output
// -----------------------------------------------------------------------------
layout (binding = 0, PANORAMA_TYPE) readonly uniform image2D in_PanoramaEstimation;
layout (binding = 1, PANORAMA_TYPE) readonly uniform image2D in_PanoramaOrginal;
layout (binding = 2, PANORAMA_TYPE) writeonly uniform image2D out_Panorama;

// -------------------------------------------------------------------------------------

layout (local_size_x = TILE_SIZE, local_size_y = TILE_SIZE, local_size_z = 1) in;
void main()
{
    ivec2 UV = ivec2(gl_GlobalInvocationID.xy);

    vec4 TexelEstimation = imageLoad(in_PanoramaEstimation, UV);

    vec4 TexelOriginal = imageLoad(in_PanoramaOrginal, UV);

    float Alpha = clamp(TexelOriginal.a * 4.0f, 0.0f, 1.0f);

    vec3 NewTexel = TexelEstimation.rgb * (1.0f - Alpha) + TexelOriginal.rgb * Alpha;

    imageStore(out_Panorama, UV, vec4(NewTexel, 1.0f));
}

#endif // __INCLUDE_CS_FUSEPANO_GLSL__