
#ifndef __INCLUDE_DS_GLOBAL_GLSL__
#define __INCLUDE_DS_GLOBAL_GLSL__

layout(row_major, std140, binding = 0) uniform UConstantBufferDS0 /* UPerFrameConstantBufferDS */
{
    mat4 ds_ViewProjectionMatrix;
};

#endif // __INCLUDE_DS_GLOBAL_GLSL__
