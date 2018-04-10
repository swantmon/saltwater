
#include "plugin_arcore/mr_precompiled.h"

#include "plugin_arcore/mr_light_estimation.h"

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