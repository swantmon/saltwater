
#ifndef __INCLUDE_COMMON_GLOBAL_GLSL__
#define __INCLUDE_COMMON_GLOBAL_GLSL__

layout(row_major, std140, binding = 0) uniform UB0
{
	mat4 ps_WorldToScreen;
    mat4 ps_WorldToQuad;
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
    vec4 ps_CameraParameters0;
};

#define ps_CameraParameterNear ps_CameraParameters0.x
#define ps_CameraParameterFar  ps_CameraParameters0.y

#endif // __INCLUDE_COMMON_GLOBAL_GLSL__