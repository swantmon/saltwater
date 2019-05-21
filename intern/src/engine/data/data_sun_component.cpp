
#include "engine/engine_precompiled.h"

#include "engine/data/data_sun_component.h"

namespace Dt
{
    REGISTER_COMPONENT_SER(CSunComponent);

    CSunComponent::CSunComponent()
        : m_RefreshMode   (Static)
        , m_Direction     (0.0f, 0.0f, -1.0f)
        , m_Color         ()
        , m_Lightness     ()
        , m_Temperature   (0.0f)
        , m_Intensity     (0.0f)
        , m_HasTemperature(false)
    {

    }

    // -----------------------------------------------------------------------------

    CSunComponent::~CSunComponent()
    {

    }

    // -----------------------------------------------------------------------------

    void CSunComponent::SetRefreshMode(ERefreshMode _RefreshMode)
    {
        m_RefreshMode = _RefreshMode;
    }

    // -----------------------------------------------------------------------------

    CSunComponent::ERefreshMode CSunComponent::GetRefreshMode()
    {
        return m_RefreshMode;
    }

    // -----------------------------------------------------------------------------

    void CSunComponent::SetColor(const glm::vec3& _rColor)
    {
        m_Color = _rColor;
    }

    // -----------------------------------------------------------------------------

    glm::vec3& CSunComponent::GetColor()  
    {
        return m_Color;
    }

    // -----------------------------------------------------------------------------

    const glm::vec3& CSunComponent::GetColor() const
    {
        return m_Color;
    }

    // -----------------------------------------------------------------------------

    void CSunComponent::SetDirection(const glm::vec3& _rDirection)
    {
        m_Direction = _rDirection;
    }

    // -----------------------------------------------------------------------------

    glm::vec3& CSunComponent::GetDirection()
    {
        return m_Direction;
    }

    // -----------------------------------------------------------------------------

    const glm::vec3& CSunComponent::GetDirection() const
    {
        return m_Direction;
    }

    // -----------------------------------------------------------------------------

    void CSunComponent::SetTemperature(float _Temperature)
    {
        m_Temperature = _Temperature;
    }

    // -----------------------------------------------------------------------------

    float CSunComponent::GetTemperature() const
    {
        return m_Temperature;
    }

    // -----------------------------------------------------------------------------

    void CSunComponent::EnableTemperature(bool _Flag)
    {
        m_HasTemperature = _Flag;
    }

    // -----------------------------------------------------------------------------

    bool CSunComponent::HasTemperature() const
    {
        return m_HasTemperature;
    }

    // -----------------------------------------------------------------------------

    void CSunComponent::SetIntensity(float _Intensity)
    {
        m_Intensity = _Intensity;
    }

    // -----------------------------------------------------------------------------

    float CSunComponent::GetIntensity() const
    {
        return m_Intensity;
    }

    // -----------------------------------------------------------------------------

    float CSunComponent::GetSunAngularRadius() const
    {
        return 0.27f * glm::pi<float>() / 180.0f;
    }

    // -----------------------------------------------------------------------------

    glm::vec3& CSunComponent::GetLightness()
    {
        return m_Lightness;
    }

    // -----------------------------------------------------------------------------

    const glm::vec3& CSunComponent::GetLightness() const
    {
        return m_Lightness;
    }

    // -----------------------------------------------------------------------------

    void CSunComponent::UpdateLightness()
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
        m_Lightness = Color * m_Intensity;
    }
} // namespace Dt