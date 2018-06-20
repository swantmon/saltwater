
#ifndef __INCLUDE_CS_HISTOGRAM_MERGE_GLSL__
#define __INCLUDE_CS_HISTOGRAM_MERGE_GLSL__

// -------------------------------------------------------------------------------------
// Defines
// -------------------------------------------------------------------------------------
#define HISTOGRAM_SIZE 64
#define TILE_SIZE      8
#define HISTORY_SIZE   8

// -----------------------------------------------------------------------------
// Input from engine
// -----------------------------------------------------------------------------
layout(std430, binding = 0) readonly buffer UHistogramPerGroupBuffer
{
    float m_HistogramPerGroup[ ];
};

layout(std430, binding = 1) writeonly buffer UHistogramBuffer
{
    float m_Histogram[HISTOGRAM_SIZE];
};

layout(std140, binding = 0) uniform UHistogramSettingBuffer
{
    uvec4 cs_ConstantData0;
    uvec4 cs_ConstantData1;
    vec4  cs_ConstantData2;
    vec4  cs_ConstantData3;
    vec4  cs_ConstantData4;
    vec4  cs_ConstantData5;
};

// -------------------------------------------------------------------------------------
// Functions
// -------------------------------------------------------------------------------------
layout( local_size_x = HISTOGRAM_SIZE, local_size_y = 1, local_size_z = 1 ) in;
void main()
{
    ivec2 cs_NumberOfThreadGroups     = ivec2(cs_ConstantData1.xy);
    float cs_ReciprocalNumberOfPixels = cs_ConstantData2.x;

    int  IndexOfPixel;
    int  ThreadGroupX;
    int  ThreadGroupY;
    float SumInBin;

    SumInBin = 0.0f;

    for (ThreadGroupY = 0; ThreadGroupY < cs_NumberOfThreadGroups.y; ++ ThreadGroupY)
    {
        for (ThreadGroupX = 0; ThreadGroupX < cs_NumberOfThreadGroups.x; ++ ThreadGroupX)
        {
            IndexOfPixel = (ThreadGroupX + ThreadGroupY * cs_NumberOfThreadGroups.x) * HISTOGRAM_SIZE + int(gl_LocalInvocationIndex);

            SumInBin += m_HistogramPerGroup[IndexOfPixel];
        }
    }

    m_Histogram[gl_LocalInvocationIndex] = SumInBin * cs_ReciprocalNumberOfPixels;
}


#endif // __INCLUDE_CS_HISTOGRAM_MERGE_GLSL__