#pragma once

#include "base/base_include_glm.h"

namespace MR
{
    class CLightEstimation
    {
    public:

        enum EEstimationState
        {
            NotValid,
            Valid,
            Undefined
        };

    public:

        CLightEstimation();
        ~CLightEstimation();

        EEstimationState GetEstimationState() const;
        float GetIntensity() const;

    protected:

        EEstimationState m_EstimationState;
        float m_Intensity;
    };
} // namespace MR