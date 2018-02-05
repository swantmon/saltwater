
#include "data/data_precompiled.h"

#include "data/data_sun_facet.h"

namespace Dt
{
    CSunLightFacet::CSunLightFacet()
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

    CSunLightFacet::~CSunLightFacet()
    {

    }

    // -----------------------------------------------------------------------------

    void CSunLightFacet::SetRefreshMode(ERefreshMode _RefreshMode)
    {
        m_RefreshMode = _RefreshMode;
    }

    // -----------------------------------------------------------------------------

    CSunLightFacet::ERefreshMode CSunLightFacet::GetRefreshMode()
    {
        return m_RefreshMode;
    }

    // -----------------------------------------------------------------------------

    void CSunLightFacet::SetColor(const glm::vec3& _rColor)
    {
        m_Color = _rColor;
    }

    // -----------------------------------------------------------------------------

    glm::vec3& CSunLightFacet::GetColor()  
    {
        return m_Color;
    }

    // -----------------------------------------------------------------------------

    const glm::vec3& CSunLightFacet::GetColor() const
    {
        return m_Color;
    }

    // -----------------------------------------------------------------------------

    void CSunLightFacet::SetDirection(const glm::vec3& _rDirection)
    {
        m_Direction = _rDirection;
    }

    // -----------------------------------------------------------------------------

    glm::vec3& CSunLightFacet::GetDirection()
    {
        return m_Direction;
    }

    // -----------------------------------------------------------------------------

    const glm::vec3& CSunLightFacet::GetDirection() const
    {
        return m_Direction;
    }

    // -----------------------------------------------------------------------------

    void CSunLightFacet::SetTemperature(float _Temperature)
    {
        m_Temperature = _Temperature;
    }

    // -----------------------------------------------------------------------------

    float CSunLightFacet::GetTemperature() const
    {
        return m_Temperature;
    }

    // -----------------------------------------------------------------------------

    void CSunLightFacet::EnableTemperature(bool _Flag)
    {
        m_HasTemperature = _Flag;
    }

    // -----------------------------------------------------------------------------

    bool CSunLightFacet::HasTemperature() const
    {
        return m_HasTemperature;
    }

    // -----------------------------------------------------------------------------

    void CSunLightFacet::SetIntensity(float _Intensity)
    {
        m_Intensity = _Intensity;
    }

    // -----------------------------------------------------------------------------

    float CSunLightFacet::GetIntensity() const
    {
        return m_Intensity;
    }

    // -----------------------------------------------------------------------------

    glm::vec3& CSunLightFacet::GetLightness()
    {
        return m_Lightness;
    }

    // -----------------------------------------------------------------------------

    const glm::vec3& CSunLightFacet::GetLightness() const
    {
        return m_Lightness;
    }

    // -----------------------------------------------------------------------------

    void CSunLightFacet::UpdateLightness()
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