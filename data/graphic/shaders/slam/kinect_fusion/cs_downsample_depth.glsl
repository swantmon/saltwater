
#ifndef __INCLUDE_CS_KINECT_DOWNSAMPLE_DEPTH_GLSL__
#define __INCLUDE_CS_KINECT_DOWNSAMPLE_DEPTH_GLSL__

// -----------------------------------------------------------------------------
// Constants
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Input from engine
// -----------------------------------------------------------------------------

layout(binding = 0, r16ui) readonly uniform uimage2D cs_InputTexture;
layout(binding = 1, r16ui) writeonly uniform uimage2D cs_OutputTexture;

// -------------------------------------------------------------------------------------
// Functions
// -------------------------------------------------------------------------------------
layout (local_size_x = TILE_SIZE2D, local_size_y = TILE_SIZE2D, local_size_z = 1) in;
void main()
{
    const vec2 InputImageSize = imageSize(cs_InputTexture);
	const vec2 OutputImageSize = imageSize(cs_OutputTexture);

	const ivec2 SamplePos = ivec2(gl_GlobalInvocationID.xy / OutputImageSize * InputImageSize);

    float Sum = 0.0f;
    int Count = 0;

    const float Center = imageLoad(cs_InputTexture, SamplePos).x;

    for (int i = -1; i < 2; ++i)
    {
        for (int j = -1; j < 2; ++j)
        {
            float Sample = imageLoad(cs_InputTexture, SamplePos + ivec2(i, j)).x;
            if (abs(Sample - Center) < TRUNCATED_DISTANCE * 1000.0f)
            {
                Sum += Sample;
                ++Count;
            }
        }
    }
    
	const int Result = int(Sum / Count);
    
    imageStore(cs_OutputTexture, ivec2(gl_GlobalInvocationID), ivec4(Result));
}

#endif // __INCLUDE_CS_KINECT_DOWNSAMPLE_DEPTH_GLSL__