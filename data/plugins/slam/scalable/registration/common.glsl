#ifndef __INCLUDE_CS_REGISTRATION_COMMON_GLSL__
#define __INCLUDE_CS_REGISTRATION_COMMON_GLSL__

layout(std140, binding = 0) uniform ConstantBuffer
{
    mat4 g_Transform;    // Transform coordinates from moving to fixed image
    mat4 g_InvTransform; // Transform coordinates from fixed to moving image
    ivec2 g_FixedImageSize;
    ivec2 g_MovingImageSize;
};

layout(std430, binding = 0) buffer Buffer
{
    vec4 g_Sum[TILE_COUNT_X * TILE_COUNT_Y];
};

float RGBToGrey(vec3 RGB)
{
	return (RGB.r + RGB.g + RGB.b) / 3.0f;
}

#define WORKGROUP_SIZE ((TILE_SIZE2D) * (TILE_SIZE2D)

layout(binding = 0, rgba8) uniform image2D FixedImage;
layout(binding = 1, rgba8) uniform image2D MovingImage;
layout(binding = 2, rg32f) uniform image2D GradientImage;

layout(binding = 0) uniform sampler2D FixedTex;
layout(binding = 1) uniform sampler2D MovingTex;
layout(binding = 2) uniform sampler2D GradientTex;

#endif //__INCLUDE_CS_REGISTRATION_COMMON_GLSL__