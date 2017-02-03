#ifndef __INCLUDE_CS_LIGHT_AREALIGHT_FILTER_GLSL__
#define __INCLUDE_CS_LIGHT_AREALIGHT_FILTER_GLSL__

// -----------------------------------------------------------------------------
// Input from engine
// -----------------------------------------------------------------------------
layout(std430, binding = 0) readonly buffer UFilterProperties
{
	vec4 cs_InverseSizeAndOffset;
    uint cs_LOD;
};

// -----------------------------------------------------------------------------
// Output
// -----------------------------------------------------------------------------
layout (binding = 0) uniform sampler2D in_Texture;
layout (binding = 1, rgba8) writeonly uniform image2D out_FilteredTexture;

// -------------------------------------------------------------------------------------
// Functions
// -------------------------------------------------------------------------------------
layout (local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main()
{
	uint PixelCoordX;
    uint PixelCoordY;
	vec4 Output;

	// Initialization
    PixelCoordX = gl_GlobalInvocationID.x;
    PixelCoordY = gl_GlobalInvocationID.y;
	
    vec2 UV =  vec2(PixelCoordX, PixelCoordY) * cs_InverseSizeAndOffset.xy;

	UV = (UV - 0.125f) * (1.0f + 0.334f);

    if (UV.x >= 0.0f && UV.y >= 0.0f && UV.x <= 1.0f && UV.y <= 1.0f)
    {
	   vec4 Output = texture(in_Texture, UV);

       imageStore(out_FilteredTexture, ivec2(PixelCoordX, PixelCoordY), Output);
    }
    else
    {
        imageStore(out_FilteredTexture, ivec2(PixelCoordX, PixelCoordY), vec4(0.125f));  
    }
}

#endif // __INCLUDE_CS_LIGHT_AREALIGHT_FILTER_GLSL__