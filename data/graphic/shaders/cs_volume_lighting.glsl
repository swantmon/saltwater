#ifndef __INCLUDE_CS_VOLUME_LIGHTING_GLSL__
#define __INCLUDE_CS_VOLUME_LIGHTING_GLSL__

layout (binding = 0, r32f) readonly  uniform image2D cs_InputImage;
layout (binding = 1, rgba32f) writeonly uniform image3D cs_OutputImage;

// -------------------------------------------------------------------------------------
// Main
// -------------------------------------------------------------------------------------

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main()
{
    uint X = gl_GlobalInvocationID.x;
    uint Y = gl_GlobalInvocationID.y;
    uint Z = gl_GlobalInvocationID.z;

    vec4 Pixel = imageLoad(cs_InputImage, ivec2(X, Y));

    imageStore(cs_OutputImage, ivec3(X, Y, Z), vec4(1,0,0,0));
}

#endif // __INCLUDE_CS_VOLUME_LIGHTING_GLSL__