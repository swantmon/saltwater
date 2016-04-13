
#pragma once

namespace MR
{
    struct SDeviceParameter
    {
        static const unsigned int s_MaximumNumberOfDistortionFactorValues = 9;
        
        int      m_FrameWidth;
        int      m_FrameHeight;
        double   m_ProjectionMatrix[3][4];
        double   m_DistortionFactor[s_MaximumNumberOfDistortionFactorValues];
        int      m_DistortionFunctionVersion;
        int      m_PixelFormat;
    };
} // namespace AR