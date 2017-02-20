
#ifndef __INCLUDE_TRACKING_COMMON_GLSL__
#define __INCLUDE_TRACKING_COMMON_GLSL__

#define PYRAMID_LEVELS 3

struct SIntrinics
{
	mat4 m_KMatrix;
	mat4 m_InvKMatrix;
	vec2 m_FocalPoint;
	vec2 m_FocalLength;
	vec2 m_InvFocalLength;
};

layout(row_major, std140, binding = 0) uniform UBOIntrinsics
{
	SIntrinics g_Intrinisics[PYRAMID_LEVELS];
};

layout(row_major, std140, binding = 1) uniform UBOTrackingData
{
	mat4 g_PoseRotationMatrix;
	mat4 g_InvPoseRotationMatrix;
	mat4 g_PoseTranslationMatrix;
	mat4 g_InvPoseTranslationMatrix;
	mat4 g_PoseMatrix;
	mat4 g_InvPoseMatrix;
};

layout(std430, binding = 0) buffer ICPBuffer
{
    float g_ICPData[ICP_VALUE_COUNT][ICP_SUMMAND_COUNT];
};

#endif // __INCLUDE_TRACKING_COMMON_GLSL__