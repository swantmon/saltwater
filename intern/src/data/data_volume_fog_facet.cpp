
#include "data/data_precompiled.h"

#include "data/data_volume_fog_facet.h"

namespace Dt
{
    CVolumeFogFXFacet::CVolumeFogFXFacet()
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

    CVolumeFogFXFacet::~CVolumeFogFXFacet()
    {

    }

    // -----------------------------------------------------------------------------

    void CVolumeFogFXFacet::SetWindDirection(const Base::Float4& _rWindDirection)
    {
        m_WindDirection = _rWindDirection;
    }

    // -----------------------------------------------------------------------------

    Base::Float4& CVolumeFogFXFacet::GetWindDirection()
    {
        return m_WindDirection;
    }

    // -----------------------------------------------------------------------------

    void CVolumeFogFXFacet::SetFogColor(const Base::Float4& _rFogColor)
    {
        m_FogColor = _rFogColor;
    }

    // -----------------------------------------------------------------------------

    Base::Float4& CVolumeFogFXFacet::GetFogColor()
    {
        return m_FogColor;
    }

    // -----------------------------------------------------------------------------

    void CVolumeFogFXFacet::SetFrustumDepthInMeter(float _FrustumDepthInMeter)
    {
        m_FrustumDepthInMeter = _FrustumDepthInMeter;
    }

    // -----------------------------------------------------------------------------

    float CVolumeFogFXFacet::GetFrustumDepthInMeter()
    {
        return m_FrustumDepthInMeter;
    }

    // -----------------------------------------------------------------------------

    void CVolumeFogFXFacet::SetShadowIntensity(float _ShadowIntensity)
    {
        m_ShadowIntensity = _ShadowIntensity;
    }

    // -----------------------------------------------------------------------------

    float CVolumeFogFXFacet::GetShadowIntensity()
    {
        return m_ShadowIntensity;
    }

    // -----------------------------------------------------------------------------

    void CVolumeFogFXFacet::SetScatteringCoefficient(float _ScatteringCoefficient)
    {
        m_ScatteringCoefficient = _ScatteringCoefficient;
    }

    // -----------------------------------------------------------------------------

    float CVolumeFogFXFacet::GetScatteringCoefficient()
    {
        return m_ScatteringCoefficient;
    }

    // -----------------------------------------------------------------------------

    void CVolumeFogFXFacet::SetAbsorptionCoefficient(float _AbsorptionCoefficient)
    {
        m_AbsorptionCoefficient = _AbsorptionCoefficient;
    }

    // -----------------------------------------------------------------------------

    float CVolumeFogFXFacet::GetAbsorptionCoefficient()
    {
        return m_AbsorptionCoefficient;
    }

    // -----------------------------------------------------------------------------

    void CVolumeFogFXFacet::SetDensityLevel(float _DensityLevel)
    {
        m_DensityLevel = _DensityLevel;
    }

    // -----------------------------------------------------------------------------

    float CVolumeFogFXFacet::GetDensityLevel()
    {
        return m_DensityLevel;
    }

    // -----------------------------------------------------------------------------

    void CVolumeFogFXFacet::SetDensityAttenuation(float _DensityAttenuation)
    {
        m_DensityAttenuation = _DensityAttenuation;
    }

    // -----------------------------------------------------------------------------

    float CVolumeFogFXFacet::GetDensityAttenuation()
    {
        return m_DensityAttenuation;
    }
} // namespace Dt