
#ifndef __INCLUDE_FS_GLOBAL_GLSL__
#define __INCLUDE_FS_GLOBAL_GLSL__

layout(row_major, std140, binding = 0) uniform UGlobalPSBuffer
{
    mat4 ps_WorldToView;
    mat4 ps_ViewToScreen;
    mat4 ps_ScreenToView;
    mat4 ps_ViewToWorld;
    vec4 ps_ViewPosition;
    vec4 ps_ViewDirection;
    mat4 ps_PreviousWorldToView;
    mat4 ps_PreviousViewToScreen;
    mat4 ps_PreviousScreenToView;
    mat4 ps_PreviousViewToWorld;
    vec4 ps_PreviousViewPosition;
    vec4 ps_PreviousViewDirection;
    vec4 ps_InvertedScreensizeAndScreensize;
    vec4 ps_ScreenPositionScaleBias;
};

#endif // __INCLUDE_FS_GLOBAL_GLSL__