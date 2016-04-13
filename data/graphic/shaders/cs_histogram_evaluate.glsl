
#ifndef __INCLUDE_CS_HISTOGRAM_EVALUATE_GLSL__
#define __INCLUDE_CS_HISTOGRAM_EVALUATE_GLSL__

// -------------------------------------------------------------------------------------
// Defines
// -------------------------------------------------------------------------------------
#define HISTOGRAM_SIZE 64
#define TILE_SIZE      8
#define HISTORY_SIZE   8

// -----------------------------------------------------------------------------
// Input from engine
// -----------------------------------------------------------------------------
layout(std430, binding = 0) readonly buffer UHistogramBuffer
{
    float m_Histogram[HISTOGRAM_SIZE];
};

layout(std430, binding = 1) buffer UExposureHistoryBuffer
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

// -------------------------------------------------------------------------------------
// Functions
// -------------------------------------------------------------------------------------
float GetLuminance(float _Bin, float _HistogramReciprocalScale, float _HistogramLogMin)
{
    return exp2(_Bin * _HistogramReciprocalScale + _HistogramLogMin);
}

// -----------------------------------------------------------------------------

float ComputeEV100(float _Aperture, float _ShutterSpeed, float _ISO)
{
    // -----------------------------------------------------------------------------
    // EV number is defined as:
    // 2^EV_s = N^2 / t and EV_s = EV_100 + log2(S/100)
    // This gives
    // EV_s = log2(N^2 / t)
    // EV_100 + log2(S/100) = log2(N^2 / t)
    // EV_100 = log2(N^2 / t) - log2(S/100)
    // EV_100 = log2(N^2 / t . 100 / S)
    // -----------------------------------------------------------------------------
    
    return log2((_Aperture * _Aperture) / _ShutterSpeed * (100.0f / _ISO));
}

// -----------------------------------------------------------------------------

float ComputeEV100FromAvgLuminance(float _AvgLuminance)
{
    // -----------------------------------------------------------------------------
    // We later use the middle gray at 12.7% in order to have
    // a middle gray at 18% with a sqrt (2) room for specular highlights
    // But here we deal with the spot meter measuring the middle gray
    // which is fixed at 12.5 for matching standard camera
    // constructor settings (i.e. calibration constant K = 12.5)
    // Reference: http ://en.wikipedia.org/wiki/Film_speed
    // -----------------------------------------------------------------------------
    
    return log2(_AvgLuminance * 100.0f / 12.5f);
}

// -----------------------------------------------------------------------------

float ConvertEV100ToExposure(float _EV100)
{
    // -----------------------------------------------------------------------------
    // Compute the maximum luminance possible with H_sbs sensitivity
    // maxLum = 78 / ( S * q ) * N^2 / t
    //        = 78 / ( S * q ) * 2^ EV_100
    //        = 78 / (100 * 0.65) * 2^ EV_100
    //        = 1.2 * 2^EV
    // Reference: http ://en.wikipedia.org/wiki/Film_speed
    // -----------------------------------------------------------------------------
    
    float MaxLuminance = 1.2f * pow(2.0f, _EV100);
    return 1.0f / MaxLuminance;
}

// -----------------------------------------------------------------------------

float Lerp(float _Start, float _End, float _Weight)
{
    return _Start + _Weight * (_End - _Start);
}

// -------------------------------------------------------------------------------------

layout( local_size_x = 1, local_size_y = 1, local_size_z = 1 ) in;
void main()
{
    uint  cs_HistoryIndex             = cs_ConstantData1.z;
    uint  cs_LastHistoryIndex         = cs_ConstantData1.w;
    float cs_Time                     = cs_ConstantData2.y;
    float cs_HistogramLowerBound      = cs_ConstantData2.z;
    float cs_HistogramUpperBound      = cs_ConstantData2.w;
    float cs_HistogramLogMin          = cs_ConstantData3.x;
    float cs_HistogramLogMax          = cs_ConstantData3.y;
    float cs_HistogramReciprocalScale = cs_ConstantData3.w;
    float cs_EyeAdaptionSpeedUp       = cs_ConstantData4.x;
    float cs_EyeAdaptionSpeedDown     = cs_ConstantData4.y;
    float cs_ExposureCompensation     = cs_ConstantData4.z;
    float cs_UseAutoExposure          = cs_ConstantData4.w;
    float cs_Aperture                 = cs_ConstantData5.x;
    float cs_Shuttertime              = cs_ConstantData5.y;
    float cs_ISO                      = cs_ConstantData5.z;
    float cs_ResetEyeAdaption         = cs_ConstantData5.w;
    
    uint   IndexOfBin;
    float  Delta;
    float  SumInBin;
    float  SumInHistogram;
    float  MinSumInHistogram;
    float  MaxSumInHistogram;
    float  LuminanceOfBin;
    vec2   RangedSumInHistogram;
    float  AverageLuminance;
    float  OldExposure;
    float  OldExposureScale;
    float  NewExposure;
    float  SmoothedExposure;
    float  SmoothedExposureScale;
    float  EyeAdaptionSpeed;
    
    SumInHistogram = 0.0f;
    
    #pragma unroll
    for (IndexOfBin = 0; IndexOfBin < HISTOGRAM_SIZE; ++ IndexOfBin)
    {
        SumInHistogram += m_Histogram[IndexOfBin];
    }
    
    // -------------------------------------------------------------------------------
    // Get the wanted exposure of the current frame.
    // -------------------------------------------------------------------------------
    MinSumInHistogram = SumInHistogram * cs_HistogramLowerBound;
    MaxSumInHistogram = SumInHistogram * cs_HistogramUpperBound;
    
    RangedSumInHistogram = vec2(0.0f);
    
    for (IndexOfBin = 0; IndexOfBin < HISTOGRAM_SIZE; ++ IndexOfBin)
    {
        SumInBin = m_Histogram[IndexOfBin];
        
        // -------------------------------------------------------------------------------
        // Remove outlier at lower bound.
        // -------------------------------------------------------------------------------
        Delta = min(SumInBin, MinSumInHistogram);
        
        SumInBin          -= Delta;
        MinSumInHistogram -= Delta;
        MaxSumInHistogram -= Delta;
        
        // -------------------------------------------------------------------------------
        // Remove outlier at upper bound.
        // -------------------------------------------------------------------------------
        SumInBin = min(SumInBin, MaxSumInHistogram);
        
        MaxSumInHistogram -= SumInBin;
        
        LuminanceOfBin = GetLuminance(float(IndexOfBin), cs_HistogramReciprocalScale, cs_HistogramLogMin);
        
        vec2 LuminanceAdd;

        LuminanceAdd.x = LuminanceOfBin;
        LuminanceAdd.y = 1.0f;

        RangedSumInHistogram += LuminanceAdd * SumInBin;
    }
    
    AverageLuminance = RangedSumInHistogram.x / max(0.0001f, RangedSumInHistogram.y);

    // -----------------------------------------------------------------------------
    // Compute exposure value
    // -----------------------------------------------------------------------------
    bool  UseAutoExposure = cs_UseAutoExposure == 1.0f;
    
    float EV100     = ComputeEV100(cs_Aperture, cs_Shuttertime, cs_ISO);
    
    float AutoEV100 = ComputeEV100FromAvgLuminance(AverageLuminance);
    
    float CurrentEV = UseAutoExposure ? AutoEV100 : EV100;

    float Exposure  = ConvertEV100ToExposure(CurrentEV - cs_ExposureCompensation);

    // -----------------------------------------------------------------------------
    // Eye adaption
    // -----------------------------------------------------------------------------
    float NextExposure = Exposure;
    
    if (cs_ResetEyeAdaption == 0.0f)
    {
        float LastExposure = m_ExposureHistory[cs_LastHistoryIndex];
        
        float AdaptionSpeed = LastExposure < Exposure ? cs_EyeAdaptionSpeedUp : cs_EyeAdaptionSpeedDown;
        
        NextExposure = Lerp(LastExposure, Exposure, cs_Time * AdaptionSpeed);
    }

    // -----------------------------------------------------------------------------
    // Save adapted exposure
    // -----------------------------------------------------------------------------
    m_ExposureHistory[cs_HistoryIndex] = NextExposure;
}

#endif // __INCLUDE_CS_HISTOGRAM_EVALUATE_GLSL__