
#ifndef __INCLUDE_VS_GLOBAL_GLSL__
#define __INCLUDE_VS_GLOBAL_GLSL__

layout(row_major, std140, binding = 0) uniform UConstantBufferVS0 /* UPerFrameConstantBufferVS */
{
    vec4 vs_InvertedScreensize;
    mat4 vs_ViewProjectionMatrix;
    mat4 vs_ViewProjectionScreenMatrix;
};

#endif // __INCLUDE_VS_GLOBAL_GLSL__