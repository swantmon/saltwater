
#ifndef __INCLUDE_TRACKING_COMMON_GLSL__
#define __INCLUDE_TRACKING_COMMON_GLSL__

layout(row_major, std140, binding = 0) uniform UBOIntrinsics
{
	mat4 g_KMatrix;
	mat4 g_InvKMatrix;
	vec2 g_FocalPoint;
	vec2 g_FocalLength;
	vec2 g_InvFocalLength;
};

layout(row_major, std430, binding = 1) buffer UBOTrackingData
{
	mat4 g_PoseRotationMatrix;
	mat4 g_InvPoseRotationMatrix;
	mat4 g_PoseTranslationMatrix;
	mat4 g_InvPoseTranslationMatrix;
	mat4 g_PoseMatrix;
	mat4 g_InvPoseMatrix;
};

#endif // __INCLUDE_TRACKING_COMMON_GLSL__