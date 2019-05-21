
#include "engine/engine_precompiled.h"

#include "base/base_include_glm.h"

#include "engine/data/data_point_light_component.h"

namespace Dt
{
    REGISTER_COMPONENT_SER(CPointLightComponent);

    CPointLightComponent::CPointLightComponent()
        : m_RefreshMode                        (Static)
        , m_ShadowType                         (HardShadows)
        , m_ShadowQuality                      (Medium)
        , m_Direction                          (0.0f, 0.01f, -1.0f)
        , m_Color                              (1.0f)
        , m_Lightness                          ()
        , m_Temperature                        (0.0f)
        , m_Intensity                          (1200.0f)
        , m_AttentuationRadius                 (20.0f)
        , m_ReciprocalSquaredAttentuationRadius(0.0f)
        , m_InnerConeAngle                     (glm::radians(60.0f))
        , m_OuterConeAngle                     (glm::radians(90.0f))
        , m_AngleScale                         (0.0f)
        , m_AngleOffset                        (1.0f)
        , m_HasTemperature                     (false)
    {
        UpdateLightness();
    }

    // -----------------------------------------------------------------------------

    CPointLightComponent::~CPointLightComponent()
    {

    }

    // -----------------------------------------------------------------------------

    void CPointLightComponent::SetShadowType(EShadowType _ShadowType)
    {
        m_ShadowType = _ShadowType;
    }

    // -----------------------------------------------------------------------------

    CPointLightComponent::EShadowType CPointLightComponent::GetShadowType()
    {
        return m_ShadowType;
    }

    // -----------------------------------------------------------------------------

    void CPointLightComponent::SetShadowQuality(EShadowQuality _ShadowQuality)
    {
        m_ShadowQuality = _ShadowQuality;
    }

    // -----------------------------------------------------------------------------

    CPointLightComponent::EShadowQuality CPointLightComponent::GetShadowQuality()
    {
        return m_ShadowQuality;
    }

    // -----------------------------------------------------------------------------

    void CPointLightComponent::SetRefreshMode(ERefreshMode _RefreshMode)
    {
        m_RefreshMode = _RefreshMode;
    }

    // -----------------------------------------------------------------------------

    CPointLightComponent::ERefreshMode CPointLightComponent::GetRefreshMode()
    {
        return m_RefreshMode;
    }

    // -----------------------------------------------------------------------------

    void CPointLightComponent::SetColor(const glm::vec3& _rColor)
    {
        m_Color = _rColor;
    }

    // -----------------------------------------------------------------------------

    glm::vec3& CPointLightComponent::GetColor()
    {
        return m_Color;
    }

    // -----------------------------------------------------------------------------

    const glm::vec3& CPointLightComponent::GetColor() const
    {
        return m_Color;
    }

    // -----------------------------------------------------------------------------

    void CPointLightComponent::SetDirection(const glm::vec3& _rDirection)
    {
        m_Direction = _rDirection;
    }

    // -----------------------------------------------------------------------------

    glm::vec3& CPointLightComponent::GetDirection()
    {
        return m_Direction;
    }

    // -----------------------------------------------------------------------------

    const glm::vec3& CPointLightComponent::GetDirection() const
    {
        return m_Direction;
    }

    // -----------------------------------------------------------------------------

    void CPointLightComponent::SetTemperature(float _Temperature)
    {
        m_Temperature = _Temperature;
    }

    // -----------------------------------------------------------------------------

    float CPointLightComponent::GetTemperature() const
    {
        return m_Temperature;
    }

    // -----------------------------------------------------------------------------

    void CPointLightComponent::EnableTemperature(bool _Flag)
    {
        m_HasTemperature = _Flag;
    }

    // -----------------------------------------------------------------------------

    bool CPointLightComponent::HasTemperature() const
    {
        return m_HasTemperature;
    }

    // -----------------------------------------------------------------------------

    void CPointLightComponent::SetIntensity(float _Intensity)
    {
        m_Intensity = _Intensity;
    }

    // -----------------------------------------------------------------------------

    float CPointLightComponent::GetIntensity() const
    {
        return m_Intensity;
    }

    // -----------------------------------------------------------------------------

    void CPointLightComponent::SetAttenuationRadius(float _AttenuationRadius)
    {
        m_AttentuationRadius = _AttenuationRadius;

        m_ReciprocalSquaredAttentuationRadius = 1.0f / (_AttenuationRadius * _AttenuationRadius);
    }

    // -----------------------------------------------------------------------------

    float CPointLightComponent::GetAttenuationRadius() const
    {
        return m_AttentuationRadius;
    }

    // -----------------------------------------------------------------------------

    float CPointLightComponent::GetReciprocalSquaredAttenuationRadius() const
    {
        return m_ReciprocalSquaredAttentuationRadius;
    }

    // -----------------------------------------------------------------------------

    void CPointLightComponent::SetInnerConeAngle(float _Angle)
    {
        m_InnerConeAngle = _Angle;
    }

    // -----------------------------------------------------------------------------

    float CPointLightComponent::GetInnerConeAngle() const
    {
        return m_InnerConeAngle;
    }

    // -----------------------------------------------------------------------------

    void CPointLightComponent::SetOuterConeAngle(float _Angle)
    {
        m_OuterConeAngle = _Angle;
    }

    // -----------------------------------------------------------------------------

    float CPointLightComponent::GetOuterConeAngle() const
    {
        return m_OuterConeAngle;
    }

    // -----------------------------------------------------------------------------

    float CPointLightComponent::GetAngleScale() const
    {
        return m_AngleScale;
    }

    // -----------------------------------------------------------------------------

    float CPointLightComponent::GetAngleOffset() const
    {
        return m_AngleOffset;
    }

    // -----------------------------------------------------------------------------

    glm::vec3& CPointLightComponent::GetLightness()
    {
        return m_Lightness;
    }

    // -----------------------------------------------------------------------------

    const glm::vec3& CPointLightComponent::GetLightness() const
    {
        return m_Lightness;
    }

    // -----------------------------------------------------------------------------

    void CPointLightComponent::UpdateLightness()
    {
        glm::vec3 Color;

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
        float CosInnerConeAngle = glm::cos(m_InnerConeAngle / 2.0f);
        float CosOuterConeAngle = glm::cos(m_OuterConeAngle / 2.0f);

        m_AngleScale  = 1.0f / glm::max(0.001f, CosInnerConeAngle - CosOuterConeAngle);
        m_AngleOffset = -CosOuterConeAngle * m_AngleScale;

        // -----------------------------------------------------------------------------
        // Luminous power (lumen) is converted into luminous intensity (candela).
        // -----------------------------------------------------------------------------
        m_Lightness = Color * (m_Intensity / glm::pi<float>());
    }
} // namespace Dt