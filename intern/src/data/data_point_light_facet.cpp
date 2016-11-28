
#include "data/data_precompiled.h"

#include "data/data_point_light_facet.h"

namespace Dt
{
    CPointLightFacet::CPointLightFacet()
        : m_RefreshMode                        (Static)
        , m_ShadowType                         (NoShadows)
        , m_ShadowQuality                      (Low)
        , m_Direction                          (0.0f, 0.0f, -1.0f)
        , m_Color                              ()
        , m_Lightness                          ()
        , m_Temperature                        (0.0f)
        , m_Intensity                          (0.0f)
        , m_AttentuationRadius                 (0.0f)
        , m_ReciprocalSquaredAttentuationRadius(0.0f)
        , m_InnerConeAngle                     (0.0f)
        , m_OuterConeAngle                     (0.0f)
        , m_AngleScale                         (0.0f)
        , m_AngleOffset                        (1.0f)
        , m_HasTemperature                     (false)
    {

    }

    // -----------------------------------------------------------------------------

    CPointLightFacet::~CPointLightFacet()
    {

    }

    // -----------------------------------------------------------------------------

    void CPointLightFacet::SetShadowType(EShadowType _ShadowType)
    {
        m_ShadowType = _ShadowType;
    }

    // -----------------------------------------------------------------------------

    CPointLightFacet::EShadowType CPointLightFacet::GetShadowType()
    {
        return m_ShadowType;
    }

    // -----------------------------------------------------------------------------

    void CPointLightFacet::SetShadowQuality(EShadowQuality _ShadowQuality)
    {
        m_ShadowQuality = _ShadowQuality;
    }

    // -----------------------------------------------------------------------------

    CPointLightFacet::EShadowQuality CPointLightFacet::GetShadowQuality()
    {
        return m_ShadowQuality;
    }

    // -----------------------------------------------------------------------------

    void CPointLightFacet::SetRefreshMode(ERefreshMode _RefreshMode)
    {
        m_RefreshMode = _RefreshMode;
    }

    // -----------------------------------------------------------------------------

    CPointLightFacet::ERefreshMode CPointLightFacet::GetRefreshMode()
    {
        return m_RefreshMode;
    }

    // -----------------------------------------------------------------------------

    void CPointLightFacet::SetColor(const Base::Float3& _rColor)
    {
        m_Color = _rColor;
    }

    // -----------------------------------------------------------------------------

    Base::Float3& CPointLightFacet::GetColor()
    {
        return m_Color;
    }

    // -----------------------------------------------------------------------------

    const Base::Float3& CPointLightFacet::GetColor() const
    {
        return m_Color;
    }

    // -----------------------------------------------------------------------------

    void CPointLightFacet::SetDirection(const Base::Float3& _rDirection)
    {
        m_Direction = _rDirection;
    }

    // -----------------------------------------------------------------------------

    Base::Float3& CPointLightFacet::GetDirection()
    {
        return m_Direction;
    }

    // -----------------------------------------------------------------------------

    const Base::Float3& CPointLightFacet::GetDirection() const
    {
        return m_Direction;
    }

    // -----------------------------------------------------------------------------

    void CPointLightFacet::SetTemperature(float _Temperature)
    {
        m_Temperature = _Temperature;
    }

    // -----------------------------------------------------------------------------

    float CPointLightFacet::GetTemperature() const
    {
        return m_Temperature;
    }

    // -----------------------------------------------------------------------------

    void CPointLightFacet::EnableTemperature(bool _Flag)
    {
        m_HasTemperature = _Flag;
    }

    // -----------------------------------------------------------------------------

    bool CPointLightFacet::HasTemperature() const
    {
        return m_HasTemperature;
    }

    // -----------------------------------------------------------------------------

    void CPointLightFacet::SetIntensity(float _Intensity)
    {
        m_Intensity = _Intensity;
    }

    // -----------------------------------------------------------------------------

    float CPointLightFacet::GetIntensity() const
    {
        return m_Intensity;
    }

    // -----------------------------------------------------------------------------

    void CPointLightFacet::SetAttenuationRadius(float _AttenuationRadius)
    {
        m_AttentuationRadius = _AttenuationRadius;

        m_ReciprocalSquaredAttentuationRadius = 1.0f / (_AttenuationRadius * _AttenuationRadius);
    }

    // -----------------------------------------------------------------------------

    float CPointLightFacet::GetAttenuationRadius() const
    {
        return m_AttentuationRadius;
    }

    // -----------------------------------------------------------------------------

    float CPointLightFacet::GetReciprocalSquaredAttenuationRadius() const
    {
        return m_ReciprocalSquaredAttentuationRadius;
    }

    // -----------------------------------------------------------------------------

    void CPointLightFacet::SetInnerConeAngle(float _Angle)
    {
        m_InnerConeAngle = _Angle;
    }

    // -----------------------------------------------------------------------------

    float CPointLightFacet::GetInnerConeAngle() const
    {
        return m_InnerConeAngle;
    }

    // -----------------------------------------------------------------------------

    void CPointLightFacet::SetOuterConeAngle(float _Angle)
    {
        m_OuterConeAngle = _Angle;
    }

    // -----------------------------------------------------------------------------

    float CPointLightFacet::GetOuterConeAngle() const
    {
        return m_OuterConeAngle;
    }

    // -----------------------------------------------------------------------------

    float CPointLightFacet::GetAngleScale() const
    {
        return m_AngleScale;
    }

    // -----------------------------------------------------------------------------

    float CPointLightFacet::GetAngleOffset() const
    {
        return m_AngleOffset;
    }

    // -----------------------------------------------------------------------------

    Base::Float3& CPointLightFacet::GetLightness()
    {
        return m_Lightness;
    }

    // -----------------------------------------------------------------------------

    const Base::Float3& CPointLightFacet::GetLightness() const
    {
        return m_Lightness;
    }

    // -----------------------------------------------------------------------------

    void CPointLightFacet::UpdateLightness()
    {
        Base::Float3 Color;

        if (m_HasTemperature)
        {
            // TODO:
            // Convert temperature to color
        }
        else
        {
            Color = m_Color;
        }

        // -----------------------------------------------------------------------------
        // Scale & Offset
        // -----------------------------------------------------------------------------
        float CosInnerConeAngle = Base::Cos(m_InnerConeAngle / 2.0f);
        float CosOuterConeAngle = Base::Cos(m_OuterConeAngle / 2.0f);

        m_AngleScale  = 1.0f / Base::Max(0.001f, CosInnerConeAngle - CosOuterConeAngle);
        m_AngleOffset = -CosOuterConeAngle * m_AngleScale;

        // -----------------------------------------------------------------------------
        // Luminous power (lumen) is converted into luminous intensity (candela).
        // -----------------------------------------------------------------------------
        m_Lightness = Color * (m_Intensity / Base::SConstants<float>::s_Pi);
    }
} // namespace Dt