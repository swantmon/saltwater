
#ifndef __INCLUDE_CS_KINECT_RAYCAST_PYRAMID_GLSL__
#define __INCLUDE_CS_KINECT_RAYCAST_PYRAMID_GLSL__

// -----------------------------------------------------------------------------
// Constants
// -----------------------------------------------------------------------------

layout(row_major, std140, binding = 0) uniform UBONormalize
{
	float g_Normalize;
};

// -----------------------------------------------------------------------------
// Input from engine

// -----------------------------------------------------------------------------
layout (binding = 0, rgba32f) readonly uniform image2D cs_InputTexture;
layout (binding = 1, rgba32f) writeonly uniform image2D cs_OutputTexture;

// Functions
// -------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------
layout (local_size_x = TILE_SIZE2D, local_size_y = TILE_SIZE2D, local_size_z = 1) in;
void main()
{
	const vec2 SamplePos = gl_GlobalInvocationID.xy * 2;
	
	vec3 Sum = vec3(0.0f);
	
    vec3 Sample00 = imageLoad(cs_InputTexture, ivec2(SamplePos.x    , SamplePos.y    )).xyz;
    vec3 Sample01 = imageLoad(cs_InputTexture, ivec2(SamplePos.x    , SamplePos.y + 1)).xyz;
    vec3 Sample10 = imageLoad(cs_InputTexture, ivec2(SamplePos.x + 1, SamplePos.y    )).xyz;
    vec3 Sample11 = imageLoad(cs_InputTexture, ivec2(SamplePos.x + 1, SamplePos.y + 1)).xyz;

    if (Sample00.x == 0.0f || Sample01.x == 0.0f || Sample10.x == 0.0f || Sample11.x == 0.0f)
    {
        imageStore(cs_OutputTexture, ivec2(gl_GlobalInvocationID), vec4(0.0f));
        return;
    }

	vec3 Result = (Sample00 + Sample01 + Sample10 + Sample11) * 0.25f;
	
	if (g_Normalize > 0.0f)
	{
		Result = normalize(Result);
	}
	
	imageStore(cs_OutputTexture, ivec2(gl_GlobalInvocationID), vec4(Result, 1.0f));
}

#endif // __INCLUDE_CS_KINECT_RAYCAST_PYRAMID_GLSL__