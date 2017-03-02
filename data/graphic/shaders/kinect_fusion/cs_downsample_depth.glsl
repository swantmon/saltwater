
#ifndef __INCLUDE_CS_KINECT_DOWNSAMPLE_DEPTH_GLSL__
#define __INCLUDE_CS_KINECT_DOWNSAMPLE_DEPTH_GLSL__

// -----------------------------------------------------------------------------
// Constants
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Input from engine
// -----------------------------------------------------------------------------

layout(binding = 0) uniform usampler2D cs_InputTexture;
layout (binding = 1, r16ui) writeonly uniform uimage2D cs_OutputTexture;

// -------------------------------------------------------------------------------------
// Functions
// -------------------------------------------------------------------------------------
layout (local_size_x = TILE_SIZE2D, local_size_y = TILE_SIZE2D, local_size_z = 1) in;
void main()
{
	const vec2 OutputImageSize = imageSize(cs_OutputTexture);

	const vec2 SamplePos = gl_GlobalInvocationID.xy / OutputImageSize;

    const vec4 DepthSamples = vec4(textureGather(cs_InputTexture, SamplePos));

	const int Result = int(dot(DepthSamples, vec4(1.0f)) * 0.25f);

    bool IsValid = DepthSamples.x != 0 && DepthSamples.y != 0 && DepthSamples.z != 0 && DepthSamples.w != 0;

    imageStore(cs_OutputTexture, ivec2(gl_GlobalInvocationID), ivec4(IsValid ? Result : 0));
}

#endif // __INCLUDE_CS_KINECT_DOWNSAMPLE_DEPTH_GLSL__