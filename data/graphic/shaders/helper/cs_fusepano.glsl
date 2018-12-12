#ifndef __INCLUDE_CS_FUSEPANO_GLSL__
#define __INCLUDE_CS_FUSEPANO_GLSL__

// #define TILE_SIZE 8
// #define ESTIMATION_TYPE rgba8
// #define ORIGINAL_TYPE rgba8

// -----------------------------------------------------------------------------
// Output
// -----------------------------------------------------------------------------
layout (binding = 0, ESTIMATION_TYPE) readonly uniform image2D in_PanoramaEstimation;
layout (binding = 1, ORIGINAL_TYPE)   uniform image2D inout_PanoramaOrginal;

// -------------------------------------------------------------------------------------

layout (local_size_x = TILE_SIZE, local_size_y = TILE_SIZE, local_size_z = 1) in;
void main()
{
    uint X;
    uint Y;

    X = gl_GlobalInvocationID.x;
    Y = gl_GlobalInvocationID.y;

    ivec2 UV;

    UV.x = abs(UV.x);
    UV.y = abs(UV.y);

    vec4 TexelEstimation = imageLoad(in_PanoramaEstimation, UV);

    vec4 TexelOriginal = imageLoad(inout_PanoramaOrginal, UV);

    float Alpha = TexelOriginal.a;

    vec3 NewTexel = TexelEstimation.rgb * (1.0f - Alpha) + TexelOriginal.rgb * Alpha;

    imageStore(inout_PanoramaOrginal, ivec2(X, Y), vec4(NewTexel, 1.0f));
}

#endif // __INCLUDE_CS_FUSEPANO_GLSL__