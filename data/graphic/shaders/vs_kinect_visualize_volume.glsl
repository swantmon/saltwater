
#ifndef __INCLUDE_VS_KINECT_VISUALIZE_VOLUME_GLSL__
#define __INCLUDE_VS_KINECT_VISUALIZE_VOLUME_GLSL__

#include "common_global.glsl"

layout (binding = 0, rg16ui) writeonly uniform uimage3D cs_Volume;

layout(row_major, std140, binding = 1) uniform PerDrawCallData
{
	mat4 g_WorldMatrix;
};

out gl_PerVertex
{
    vec4 gl_Position;
};

void main(void)
{
    gl_Position = vec4(0.0, 0.0, 0.0, 1.0);
}

#endif // __INCLUDE_VS_KINECT_VISUALIZE_VOLUME_GLSL__