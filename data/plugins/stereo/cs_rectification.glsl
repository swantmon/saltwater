#ifndef __INCLUDE_CS_YUV_TO_RGB_GLSL__
#define __INCLUDE_CS_YUV_TO_RGB_GLSL__

layout(std140, binding = 0) uniform HomographyBuffer
{
    mat3 g_Homography;
};

layout (binding = 0, r8) readonly uniform image2D cs_InputImage;
layout (binding = 1, r8ui) writeonly uniform uimage2D cs_OutputImage;

layout (local_size_x = TILE_SIZE_2D, local_size_y = TILE_SIZE_2D, local_size_z = 1) in;
void main()
{
    //imageStore(cs_OutputImage, ivec2(gl_GlobalInvocationID.xy), uvec4(255));
}

#endif //__INCLUDE_CS_YUV_TO_RGB_GLSL__