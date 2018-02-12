
#ifndef __INCLUDE_CS_HISTOGRAM_BUILD_GLSL__
#define __INCLUDE_CS_HISTOGRAM_BUILD_GLSL__

// -------------------------------------------------------------------------------------
// Defines
// -------------------------------------------------------------------------------------
#define HISTOGRAM_SIZE      64
#define TILE_SIZE           8
#define THREAD_GROUP_SIZE_X 8
#define THREAD_GROUP_SIZE_Y 8
#define HISTORY_SIZE        8

// -----------------------------------------------------------------------------
// Input from engine
// -----------------------------------------------------------------------------
layout(std430, binding = 0) writeonly buffer UHistogramPerGroupBuffer
{
    float m_HistogramPerGroup[ ];
};

layout(std430, binding = 2) readonly buffer UExposureHistoryBuffer
{
    float m_ExposureHistory[HISTORY_SIZE];
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

layout (binding = 0, rgba16f)  readonly uniform image2D PSTextureInput;

// -----------------------------------------------------------------------------
// Local
// -----------------------------------------------------------------------------
shared float g_Histograms[THREAD_GROUP_SIZE_X][THREAD_GROUP_SIZE_Y][HISTOGRAM_SIZE];

// -------------------------------------------------------------------------------------
// Functions
// -------------------------------------------------------------------------------------
float GetBin(float _Luminance, float _HistogramLogMin, float _HistogramLogMax, float _HistogramScale)
{
    float Position;

    Position = clamp(log2(_Luminance), _HistogramLogMin, _HistogramLogMax) - _HistogramLogMin;

    return Position * _HistogramScale;
}

// -------------------------------------------------------------------------------------

layout( local_size_x = THREAD_GROUP_SIZE_X, local_size_y = THREAD_GROUP_SIZE_Y, local_size_z = 1 ) in;
void main()
{
    int    IndexOfPixel;
    int    IndexOfBin;
    int    IndexOfBin0;
    int    IndexOfBin1;
    int    PixelX;
    int    PixelY;
    ivec2  PixelCoord;
    ivec2  PixelOffset;
    int    ThreadX;
    int    ThreadY;
    float  Exposure;
    vec4   LuminanceRGBA;
    vec3   LuminanceRGB;
    float  Luminance;
    float  Bin;
    float  Weight0;
    float  Weight1;
    
    ivec2 cs_MinPixelCoord        = ivec2(cs_ConstantData0.xy);
    ivec2 cs_MaxPixelCoord        = ivec2(cs_ConstantData0.zw);
    ivec2 cs_NumberOfThreadGroups = ivec2(cs_ConstantData1.xy);
    int   cs_LastHistoryIndex     = int(cs_ConstantData1.w);
    float cs_HistogramLogMin      = cs_ConstantData3.x;
    float cs_HistogramLogMax      = cs_ConstantData3.y;
    float cs_HistogramScale       = cs_ConstantData3.z;
        
    // -------------------------------------------------------------------------------------
    // Local data
    // -------------------------------------------------------------------------------------    
    #pragma unroll
    for (IndexOfBin = 0; IndexOfBin < HISTOGRAM_SIZE; ++ IndexOfBin)
    {
        g_Histograms[gl_LocalInvocationID.x][gl_LocalInvocationID.y][IndexOfBin] = 0.0f;
    }
    
    // -----------------------------------------------------------------------------
    // Average exposure
    // -----------------------------------------------------------------------------
    float AverageExposure = m_ExposureHistory[cs_LastHistoryIndex];
    
    // -------------------------------------------------------------------------------------
    // Calculate histogram of this thread
    // -------------------------------------------------------------------------------------
    PixelOffset = cs_MinPixelCoord + ivec2(gl_GlobalInvocationID.xy) * ivec2(TILE_SIZE, TILE_SIZE);
    
    for (PixelY = 0; PixelY < TILE_SIZE; ++ PixelY)
    {
        for (PixelX = 0; PixelX < TILE_SIZE; ++ PixelX)
        {            
            PixelCoord = PixelOffset + ivec2(PixelX, PixelY);
            
            bvec2 IsInside;

            IsInside.x = PixelCoord.x < cs_MaxPixelCoord.x;
            IsInside.y = PixelCoord.y < cs_MaxPixelCoord.y;

            if (all(IsInside))
            {
                vec3 LuminanceDot;
                
                LuminanceDot.x = 0.3f; // 0.299f;
                LuminanceDot.y = 0.59f; // 0.587f;
                LuminanceDot.z = 0.11f; // 0.114f;

                LuminanceRGBA = imageLoad(PSTextureInput, ivec2(PixelCoord.x, PixelCoord.y));

                LuminanceRGB  = LuminanceRGBA.xyz / AverageExposure;
                Luminance     = dot(LuminanceRGB, LuminanceDot);
                
                Bin = GetBin(Luminance, cs_HistogramLogMin, cs_HistogramLogMax, cs_HistogramScale);
                
                IndexOfBin0 = int(Bin);
                IndexOfBin1 = IndexOfBin0 + 1;
                
                Weight1 = Bin - floor(Bin);
                Weight0 = 1.0f - Weight1;
                
                if (IndexOfBin0 != 0)
                {
                    g_Histograms[gl_LocalInvocationID.x][gl_LocalInvocationID.y][IndexOfBin0] += Weight0;    
                }
                g_Histograms[gl_LocalInvocationID.x][gl_LocalInvocationID.y][IndexOfBin1] += Weight1;
            }
        }
    }
    
    // -------------------------------------------------------------------------------------
    // Put data from local histogram into histogram per thread
    // -------------------------------------------------------------------------------------
    memoryBarrier();

    float SumInBin = 0.0f;

    for (ThreadY = 0; ThreadY < THREAD_GROUP_SIZE_Y; ++ ThreadY)
    {
        for (ThreadX = 0; ThreadX < THREAD_GROUP_SIZE_X; ++ ThreadX)
        {
            SumInBin += g_Histograms[ThreadX][ThreadY][gl_LocalInvocationIndex];
        }
    }

    IndexOfPixel = (int(gl_WorkGroupID.x) + int(gl_WorkGroupID.y) * cs_NumberOfThreadGroups.x) * HISTOGRAM_SIZE + int(gl_LocalInvocationIndex);

    m_HistogramPerGroup[IndexOfPixel] = SumInBin;
}

#endif // __INCLUDE_CS_HISTOGRAM_BUILD_GLSL__