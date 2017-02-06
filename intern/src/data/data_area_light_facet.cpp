
#include "data/data_precompiled.h"

#include "data/data_area_light_facet.h"

namespace Dt
{
    CAreaLightFacet::CAreaLightFacet()
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
        , m_pTexture      (0)
    {

    }

    // -----------------------------------------------------------------------------

    CAreaLightFacet::~CAreaLightFacet()
    {

    }

    // -----------------------------------------------------------------------------

    void CAreaLightFacet::SetColor(const Base::Float3& _rColor)
    {
        m_Color = _rColor;
    }

    // -----------------------------------------------------------------------------

    Base::Float3& CAreaLightFacet::GetColor()
    {
        return m_Color;
    }

    // -----------------------------------------------------------------------------

    const Base::Float3& CAreaLightFacet::GetColor() const
    {
        return m_Color;
    }

    // -----------------------------------------------------------------------------

    void CAreaLightFacet::SetTexture(Dt::CTexture2D* _pTexture2D)
    {
        m_pTexture = _pTexture2D;
    }

    // -----------------------------------------------------------------------------

    Dt::CTexture2D* CAreaLightFacet::GetTexture()
    {
        return m_pTexture;
    }

    // -----------------------------------------------------------------------------

    bool CAreaLightFacet::GetHasTexture() const
    {
        return m_pTexture != 0;
    }

    // -----------------------------------------------------------------------------

    void CAreaLightFacet::SetDirection(const Base::Float3& _rDirection)
    {
        m_Direction = _rDirection;
    }

    // -----------------------------------------------------------------------------

    Base::Float3& CAreaLightFacet::GetDirection()
    {
        return m_Direction;
    }

    // -----------------------------------------------------------------------------

    const Base::Float3& CAreaLightFacet::GetDirection() const
    {
        return m_Direction;
    }

    // -----------------------------------------------------------------------------

    void CAreaLightFacet::SetTemperature(float _Temperature)
    {
        m_Temperature = _Temperature;
    }

    // -----------------------------------------------------------------------------

    float CAreaLightFacet::GetTemperature() const
    {
        return m_Temperature;
    }

    // -----------------------------------------------------------------------------

    void CAreaLightFacet::EnableTemperature(bool _Flag)
    {
        m_HasTemperature = _Flag;
    }

    // -----------------------------------------------------------------------------

    bool CAreaLightFacet::HasTemperature() const
    {
        return m_HasTemperature;
    }

    // -----------------------------------------------------------------------------

    void CAreaLightFacet::SetIntensity(float _Intensity)
    {
        m_Intensity = _Intensity;
    }

    // -----------------------------------------------------------------------------

    float CAreaLightFacet::GetIntensity() const
    {
        return m_Intensity;
    }

    // -----------------------------------------------------------------------------

    void CAreaLightFacet::SetRotation(float _Rotation)
    {
        m_Rotation = _Rotation;
    }

    // -----------------------------------------------------------------------------

    float CAreaLightFacet::GetRotation() const
    {
        return m_Rotation;
    }

    // -----------------------------------------------------------------------------

    void CAreaLightFacet::SetWidth(float _Angle)
    {
        m_Width = _Angle;
    }

    // -----------------------------------------------------------------------------

    float CAreaLightFacet::GetWidth() const
    {
        return m_Width;
    }

    // -----------------------------------------------------------------------------

    void CAreaLightFacet::SetHeight(float _Angle)
    {
        m_Height = _Angle;
    }

    // -----------------------------------------------------------------------------

    float CAreaLightFacet::GetHeight() const
    {
        return m_Height;
    }

    // -----------------------------------------------------------------------------

    void CAreaLightFacet::SetIsTwoSided(bool _Flag)
    {
        m_IsTwoSided = _Flag;
    }

    // -----------------------------------------------------------------------------

    bool CAreaLightFacet::GetIsTwoSided() const
    {
        return m_IsTwoSided;
    }

    // -----------------------------------------------------------------------------

    Base::Float3& CAreaLightFacet::GetLightness()
    {
        return m_Lightness;
    }

    // -----------------------------------------------------------------------------

    const Base::Float3& CAreaLightFacet::GetLightness() const
    {
        return m_Lightness;
    }

    // -----------------------------------------------------------------------------

    void CAreaLightFacet::UpdateLightness()
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
        // Luminous power (lumen) is converted into luminous intensity (candela).
        // -----------------------------------------------------------------------------
        m_Lightness = Color * (m_Intensity / (m_Width * m_Height * Base::SConstants<float>::s_Pi));
    }
} // namespace Dt