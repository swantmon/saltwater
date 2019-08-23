#ifndef __INCLUDE_CS_REGISTRATION_COMMON_GLSL__
#define __INCLUDE_CS_REGISTRATION_COMMON_GLSL__

layout(binding = 0, rgba8) uniform image2D cs_Target;
layout(binding = 1, rgba8) uniform image2D cs_Source;
layout(binding = 2, rgba32f) uniform image2D cs_SSD;
layout(binding = 3, rgba32f) uniform image2D cs_Gradient;

#endif //__INCLUDE_CS_REGISTRATION_COMMON_GLSL__