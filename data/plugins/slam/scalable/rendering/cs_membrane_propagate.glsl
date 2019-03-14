#ifndef __INCLUDE_CS_YUV_TO_RGB_GLSL__
#define __INCLUDE_CS_YUV_TO_RGB_GLSL__

layout (binding = 0, rgba8) uniform image2D cs_Diminished;
layout (binding = 1, rgba8) uniform image2D cs_Background;
layout (binding = 2, rgba16f) uniform image2D cs_MembranePatches;
layout (binding = 3, rgba16f) uniform image2D cs_MembraneBorders;

layout (local_size_x = MAX_BORDER_PATCH_COUNT, local_size_y = 1, local_size_z = 1) in;
void main()
{
    
}

#endif //__INCLUDE_CS_YUV_TO_RGB_GLSL__