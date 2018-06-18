
#ifndef __INCLUDE_FS_LIGHT_AREALIGHT_BULB_GLSL_
#define __INCLUDE_FS_LIGHT_AREALIGHT_BULB_GLSL_

// -----------------------------------------------------------------------------
// Input from engine
// -----------------------------------------------------------------------------
layout(std140, binding = 1) uniform UB1
{
    vec4  m_Color;
    vec4  m_Position;
    vec4  m_DirectionX;
    vec4  m_DirectionY;
    vec4  m_Plane;
    float m_HalfWidth;
    float m_HalfHeight;
    float m_IsTwoSided;
    float m_IsTextured;
    uint  m_ExposureHistoryIndex;
};

layout(std430, binding = 0) readonly buffer BB0
{
    float ps_ExposureHistory[8];
};

layout(binding = 0) uniform sampler2D ps_Map;

// -----------------------------------------------------------------------------
// Output to fragment
// -----------------------------------------------------------------------------
layout(location = 2) in vec2 in_UV;

// -----------------------------------------------------------------------------
// Output to fragment
// -----------------------------------------------------------------------------
layout(location = 0) out vec4 out_Color;

// -----------------------------------------------------------------------------
// Main
// -----------------------------------------------------------------------------
void main(void)
{
	vec4 Output = vec4(m_Color.xyz, 1.0f);

	if (m_IsTextured > 0.0f)
	{
		Output *= texture(ps_Map, in_UV);
	} 

    out_Color = Output * m_Color.w * ps_ExposureHistory[m_ExposureHistoryIndex] * 0.01f;
}

#endif // __INCLUDE_FS_LIGHT_AREALIGHT_BULB_GLSL_