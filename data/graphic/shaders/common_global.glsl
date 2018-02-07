
#ifndef __INCLUDE_COMMON_GLOBAL_GLSL__
#define __INCLUDE_COMMON_GLOBAL_GLSL__

layout(std140, binding = 0) uniform UB0
{
	mat4 g_WorldToScreen;
    mat4 g_WorldToQuad;
    mat4 g_WorldToView;
    mat4 g_ViewToScreen;
    mat4 g_ScreenToView;
    mat4 g_ViewToWorld;
    vec4 g_ViewPosition;
    vec4 g_ViewDirection;
    mat4 g_PreviousWorldToView;
    mat4 g_PreviousViewToScreen;
    mat4 g_PreviousScreenToView;
    mat4 g_PreviousViewToWorld;
    vec4 g_PreviousViewPosition;
    vec4 g_PreviousViewDirection;
    vec4 g_InvertedScreensizeAndScreensize;
    vec4 g_ScreenPositionScaleBias;
    vec4 g_CameraParameters0;
    vec4 g_WorldParameters0;
    vec4 g_FrameParameters0;
};

#define g_CameraParameterNear g_CameraParameters0.x
#define g_CameraParameterFar  g_CameraParameters0.y

#define g_WorldSizeX g_WorldParameters0.x
#define g_WorldSizeY g_WorldParameters0.y
#define g_WorldSizeZ g_WorldParameters0.z

#define g_FrameNumber    g_FrameParameters0.x
#define g_FrameDeltaTime g_FrameParameters0.y

#endif // __INCLUDE_COMMON_GLOBAL_GLSL__