
#include "mr/mr_precompiled.h"

#include "mr/mr_light_estimation.h"

namespace MR
{
    CLightEstimation::CLightEstimation()
        : m_EstimationState(Undefined)
        , m_Intensity      (0.8f)
    {

    }

    // -----------------------------------------------------------------------------

    CLightEstimation::~CLightEstimation()
    {

    }

    // -----------------------------------------------------------------------------

    CLightEstimation::EEstimationState CLightEstimation::GetEstimationState() const
    {
        return m_EstimationState;
    }

    // -----------------------------------------------------------------------------

    float CLightEstimation::GetIntensity() const
    {
        return m_Intensity;
    }
} // namespace MR