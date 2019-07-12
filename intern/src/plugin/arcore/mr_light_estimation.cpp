
#include "plugin/arcore/mr_precompiled.h"

#include "engine/core/core_plugin.h"

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

extern "C" CORE_PLUGIN_API_EXPORT MR::CLightEstimation::EEstimationState GetLightEstimationState(const MR::CLightEstimation* _pObj)
{
    return _pObj->GetEstimationState();
}

extern "C" CORE_PLUGIN_API_EXPORT float GetLightEstimationPixelIntensity(const MR::CLightEstimation* _pObj)
{
    return _pObj->GetPixelIntensity();
}

extern "C" CORE_PLUGIN_API_EXPORT glm::vec4 GetLightEstimationColorCorrection(const MR::CLightEstimation* _pObj)
{
    return _pObj->GetColorCorrection();
}

extern "C" CORE_PLUGIN_API_EXPORT glm::vec3 GetLightEstimationMainLightIntensity(const MR::CLightEstimation* _pObj)
{
    return _pObj->GetMainLightIntensity();
}

extern "C" CORE_PLUGIN_API_EXPORT glm::vec3 GetLightEstimationMainLightDirection(const MR::CLightEstimation* _pObj)
{
    return _pObj->GetMainLightDirection();
}

extern "C" CORE_PLUGIN_API_EXPORT const float* GetLightEstimationAmbientSHCoefficients(const MR::CLightEstimation* _pObj)
{
    return _pObj->GetAmbientSHCoefficients();
}

extern "C" CORE_PLUGIN_API_EXPORT const void* GetLightEstimationHDRCubemap(const MR::CLightEstimation* _pObj)
{
    return &_pObj->GetHDRCubemap();
}