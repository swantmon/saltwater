
#ifndef __INCLUDE_FS_LIGHT_AREALIGHT_BULB_GLSL_
#define __INCLUDE_FS_LIGHT_AREALIGHT_BULB_GLSL_

// -----------------------------------------------------------------------------
// Input from engine
// -----------------------------------------------------------------------------
layout(std140, binding = 1) uniform UB1
{
    vec4  ps_Color;
};

// -----------------------------------------------------------------------------
// Output to fragment
// -----------------------------------------------------------------------------
layout(location = 0) out vec4 out_Color;

void main(void)
{
    out_Color = vec4(ps_Color);
}

#endif // __INCLUDE_FS_LIGHT_AREALIGHT_BULB_GLSL_