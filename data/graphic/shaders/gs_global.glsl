
#ifndef __INCLUDE_GS_GLOBAL_GLSL__
#define __INCLUDE_GS_GLOBAL_GLSL__

layout(row_major, std140, binding = 0) uniform UConstantBufferGS0 /* UPerFrameConstantBufferGS */
{
    mat4 gs_ViewProjectionScreenMatrix;
};

#endif // __INCLUDE_GS_GLOBAL_GLSL__