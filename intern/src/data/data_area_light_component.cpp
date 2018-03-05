
#include "data/data_precompiled.h"

#include "data/data_area_light_component.h"

#include <string>

namespace Dt
{
    CAreaLightComponent::CAreaLightComponent()
        : m_Direction     (0.01f, 0.01f, -1.0f)
        , m_Color         ()
        , m_Lightness     ()
        , m_Temperature   (0.0f)
        , m_Intensity     (0.0f)
        , m_Rotation      (0.0f)
        , m_Width         (0.0f)
        , m_Height        (0.0f)
        , m_IsTwoSided    (false)
        , m_HasTemperature(false)
        , m_Texture       ("")
    {

    }

    // -----------------------------------------------------------------------------

    CAreaLightComponent::~CAreaLightComponent()
    {

    }

    // -----------------------------------------------------------------------------

    void CAreaLightComponent::SetColor(const glm::vec3& _rColor)
    {
        m_Color = _rColor;
    }

    // -----------------------------------------------------------------------------

    glm::vec3& CAreaLightComponent::GetColor()
    {
        return m_Color;
    }

    // -----------------------------------------------------------------------------

    const glm::vec3& CAreaLightComponent::GetColor() const
    {
        return m_Color;
    }

    // -----------------------------------------------------------------------------

    void CAreaLightComponent::SetTexture(const std::string& _rTexture2D)
    {
        m_Texture = _rTexture2D;
    }

    // -----------------------------------------------------------------------------

    const std::string& CAreaLightComponent::GetTexture()
    {
        return m_Texture;
    }

    // -----------------------------------------------------------------------------

    bool CAreaLightComponent::GetHasTexture() const
    {
        return m_Texture.length() > 0;
    }

    // -----------------------------------------------------------------------------

    void CAreaLightComponent::SetDirection(const glm::vec3& _rDirection)
    {
        m_Direction = _rDirection;
    }

    // -----------------------------------------------------------------------------

    glm::vec3& CAreaLightComponent::GetDirection()
    {
        return m_Direction;
    }

    // -----------------------------------------------------------------------------

    const glm::vec3& CAreaLightComponent::GetDirection() const
    {
        return m_Direction;
    }

    // -----------------------------------------------------------------------------

    void CAreaLightComponent::SetTemperature(float _Temperature)
    {
        m_Temperature = _Temperature;
    }

    // -----------------------------------------------------------------------------

    float CAreaLightComponent::GetTemperature() const
    {
        return m_Temperature;
    }

    // -----------------------------------------------------------------------------

    void CAreaLightComponent::EnableTemperature(bool _Flag)
    {
        m_HasTemperature = _Flag;
    }

    // -----------------------------------------------------------------------------

    bool CAreaLightComponent::HasTemperature() const
    {
        return m_HasTemperature;
    }

    // -----------------------------------------------------------------------------

    void CAreaLightComponent::SetIntensity(float _Intensity)
    {
        m_Intensity = _Intensity;
    }

    // -----------------------------------------------------------------------------

    float CAreaLightComponent::GetIntensity() const
    {
        return m_Intensity;
    }

    // -----------------------------------------------------------------------------

    void CAreaLightComponent::SetRotation(float _Rotation)
    {
        m_Rotation = _Rotation;
    }

    // -----------------------------------------------------------------------------

    float CAreaLightComponent::GetRotation() const
    {
        return m_Rotation;
    }

    // -----------------------------------------------------------------------------

    void CAreaLightComponent::SetWidth(float _Angle)
    {
        m_Width = _Angle;
    }

    // -----------------------------------------------------------------------------

    float CAreaLightComponent::GetWidth() const
    {
        return m_Width;
    }

    // -----------------------------------------------------------------------------

    void CAreaLightComponent::SetHeight(float _Angle)
    {
        m_Height = _Angle;
    }

    // -----------------------------------------------------------------------------

    float CAreaLightComponent::GetHeight() const
    {
        return m_Height;
    }

    // -----------------------------------------------------------------------------

    void CAreaLightComponent::SetIsTwoSided(bool _Flag)
    {
        m_IsTwoSided = _Flag;
    }

    // -----------------------------------------------------------------------------

    bool CAreaLightComponent::GetIsTwoSided() const
    {
        return m_IsTwoSided;
    }

    // -----------------------------------------------------------------------------

    glm::vec3& CAreaLightComponent::GetLightness()
    {
        return m_Lightness;
    }

    // -----------------------------------------------------------------------------

    const glm::vec3& CAreaLightComponent::GetLightness() const
    {
        return m_Lightness;
    }

    // -----------------------------------------------------------------------------

    void CAreaLightComponent::UpdateLightness()
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
        // Luminous power (lumen) is converted into luminous intensity (candela).
        // -----------------------------------------------------------------------------
        m_Lightness = Color * (m_Intensity / (m_Width * m_Height * glm::pi<float>()));
    }
} // namespace Dt