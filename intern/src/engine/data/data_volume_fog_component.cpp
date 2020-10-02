
#include "engine/engine_precompiled.h"

#include "engine/data/data_volume_fog_component.h"

namespace Dt
{
    REGISTER_COMPONENT_SER(CVolumeFogComponent);

    CVolumeFogComponent::CVolumeFogComponent()
        : m_WindDirection        (0.0f)
        , m_FogColor             (1.0f)
        , m_FrustumDepthInMeter  (32.0f)
        , m_ShadowIntensity      (1.0f)
        , m_ScatteringCoefficient(0.05f)
        , m_AbsorptionCoefficient(0.01f)
        , m_DensityLevel         (0.8f)
        , m_DensityAttenuation   (0.5f)
    {

    }

    // -----------------------------------------------------------------------------

    CVolumeFogComponent::~CVolumeFogComponent()
    {

    }

    // -----------------------------------------------------------------------------

    void CVolumeFogComponent::SetWindDirection(const glm::vec4& _rWindDirection)
    {
        m_WindDirection = _rWindDirection;
    }

    // -----------------------------------------------------------------------------

    glm::vec4& CVolumeFogComponent::GetWindDirection()
    {
        return m_WindDirection;
    }

    // -----------------------------------------------------------------------------

    void CVolumeFogComponent::SetFogColor(const glm::vec4& _rFogColor)
    {
        m_FogColor = _rFogColor;
    }

    // -----------------------------------------------------------------------------

    glm::vec4& CVolumeFogComponent::GetFogColor()
    {
        return m_FogColor;
    }

    // -----------------------------------------------------------------------------

    void CVolumeFogComponent::SetFrustumDepthInMeter(float _FrustumDepthInMeter)
    {
        m_FrustumDepthInMeter = _FrustumDepthInMeter;
    }

    // -----------------------------------------------------------------------------

    float CVolumeFogComponent::GetFrustumDepthInMeter()
    {
        return m_FrustumDepthInMeter;
    }

    // -----------------------------------------------------------------------------

    void CVolumeFogComponent::SetShadowIntensity(float _ShadowIntensity)
    {
        m_ShadowIntensity = _ShadowIntensity;
    }

    // -----------------------------------------------------------------------------

    float CVolumeFogComponent::GetShadowIntensity()
    {
        return m_ShadowIntensity;
    }

    // -----------------------------------------------------------------------------

    void CVolumeFogComponent::SetScatteringCoefficient(float _ScatteringCoefficient)
    {
        m_ScatteringCoefficient = _ScatteringCoefficient;
    }

    // -----------------------------------------------------------------------------

    float CVolumeFogComponent::GetScatteringCoefficient()
    {
        return m_ScatteringCoefficient;
    }

    // -----------------------------------------------------------------------------

    void CVolumeFogComponent::SetAbsorptionCoefficient(float _AbsorptionCoefficient)
    {
        m_AbsorptionCoefficient = _AbsorptionCoefficient;
    }

    // -----------------------------------------------------------------------------

    float CVolumeFogComponent::GetAbsorptionCoefficient()
    {
        return m_AbsorptionCoefficient;
    }

    // -----------------------------------------------------------------------------

    void CVolumeFogComponent::SetDensityLevel(float _DensityLevel)
    {
        m_DensityLevel = _DensityLevel;
    }

    // -----------------------------------------------------------------------------

    float CVolumeFogComponent::GetDensityLevel()
    {
        return m_DensityLevel;
    }

    // -----------------------------------------------------------------------------

    void CVolumeFogComponent::SetDensityAttenuation(float _DensityAttenuation)
    {
        m_DensityAttenuation = _DensityAttenuation;
    }

    // -----------------------------------------------------------------------------

    float CVolumeFogComponent::GetDensityAttenuation()
    {
        return m_DensityAttenuation;
    }
} // namespace Dt