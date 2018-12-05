
#ifndef __INCLUDE_CS_CUBE2PANO_GLSL__
#define __INCLUDE_CS_CUBE2PANO_GLSL__

// #define TILE_SIZE 8
// #define IMAGE_TYPE rgba8

// -----------------------------------------------------------------------------
// Output
// -----------------------------------------------------------------------------
layout (binding = 0, IMAGE_TYPE) readonly uniform image2D in_Cubemap;
layout (binding = 1, IMAGE_TYPE) readonly uniform image2D out_Panorama;

// -------------------------------------------------------------------------------------

layout (local_size_x = TILE_SIZE, local_size_y = TILE_SIZE, local_size_z = 1) in;
void main()
{
    uint  X;
    uint  Y;

    X = gl_GlobalInvocationID.x;
    Y = gl_GlobalInvocationID.y;
 
    imageStore(out_Panorama, ivec2(X, Y), vec4(255, 0, 0, 1.0f));
}

#endif // __INCLUDE_CS_CUBE2PANO_GLSL__