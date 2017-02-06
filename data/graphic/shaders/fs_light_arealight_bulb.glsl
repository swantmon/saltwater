
#ifndef __INCLUDE_FS_LIGHT_AREALIGHT_BULB_GLSL_
#define __INCLUDE_FS_LIGHT_AREALIGHT_BULB_GLSL_

// -----------------------------------------------------------------------------
// Input from engine
// -----------------------------------------------------------------------------
layout(std140, binding = 1) uniform UB1
{
    vec4  ps_Color;
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
	vec4 Output = vec4(ps_Color.xyz, 1.0f);

	if (ps_Color.w > 0.0f)
	{
		Output *= texture(ps_Map, in_UV);
	} 

    out_Color = Output;
}

#endif // __INCLUDE_FS_LIGHT_AREALIGHT_BULB_GLSL_