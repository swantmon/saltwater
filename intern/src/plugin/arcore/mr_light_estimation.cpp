
#include "plugin/arcore/mr_precompiled.h"

#include "plugin/arcore/mr_light_estimation.h"

namespace MR
{
    CLightEstimation::CLightEstimation()
        : m_EstimationState   (Undefined)
        , m_PixelIntensity    (1.0f)
        , m_ColorCorrection   (0.0f)
        , m_MainLightIntensity(1.0f)
        , m_MainLightDirection(0.0f, 0.0f, -1.0f)
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

    float CLightEstimation::GetPixelIntensity() const
    {
        return m_PixelIntensity;
    }

    // -----------------------------------------------------------------------------

    const glm::vec4& CLightEstimation::GetColorCorrection() const
    {
        return m_ColorCorrection;
    }

    // -----------------------------------------------------------------------------

    const glm::vec3& CLightEstimation::GetMainLightIntensity() const
    {
        return m_MainLightIntensity;
    }

    // -----------------------------------------------------------------------------

    const glm::vec3& CLightEstimation::GetMainLightDirection() const
    {
        return m_MainLightDirection;
    }

    // -----------------------------------------------------------------------------

    const float* CLightEstimation::GetAmbientSHCoefficients() const
    {
        return m_AmbientSH;
    }

    // -----------------------------------------------------------------------------

    const ArImageCubemap& CLightEstimation::GetHDRCubemap() const
    {
        return m_HDRCubemap;
    }
} // namespace MR