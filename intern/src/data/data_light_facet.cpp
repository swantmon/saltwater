
#include "data/data_light_facet.h"

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

    void CSunLightFacet::SetColor(const Base::Float3& _rColor)
    {
        m_Color = _rColor;
    }

    // -----------------------------------------------------------------------------

    Base::Float3& CSunLightFacet::GetColor()  
    {
        return m_Color;
    }

    // -----------------------------------------------------------------------------

    const Base::Float3& CSunLightFacet::GetColor() const
    {
        return m_Color;
    }

    // -----------------------------------------------------------------------------

    void CSunLightFacet::SetDirection(const Base::Float3& _rDirection)
    {
        m_Direction = _rDirection;
    }

    // -----------------------------------------------------------------------------

    Base::Float3& CSunLightFacet::GetDirection()
    {
        return m_Direction;
    }

    // -----------------------------------------------------------------------------

    const Base::Float3& CSunLightFacet::GetDirection() const
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

    Base::Float3& CSunLightFacet::GetLightness()
    {
        return m_Lightness;
    }

    // -----------------------------------------------------------------------------

    const Base::Float3& CSunLightFacet::GetLightness() const
    {
        return m_Lightness;
    }

    // -----------------------------------------------------------------------------

    void CSunLightFacet::UpdateLightness()
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
        m_Lightness = Color * m_Intensity;
    }
} // namespace Dt

namespace Dt
{
    CGlobalProbeLightFacet::CGlobalProbeLightFacet()
        : m_Type       (Sky)
        , m_Quality    (PX128)
        , m_pCubemap   ()
        , m_Intensity  (0)
    {

    }

    // -----------------------------------------------------------------------------

    CGlobalProbeLightFacet::~CGlobalProbeLightFacet()
    {

    }

    // -----------------------------------------------------------------------------

    void CGlobalProbeLightFacet::SetType(EType _Type)
    {
        m_Type = _Type;
    }

    // -----------------------------------------------------------------------------

    CGlobalProbeLightFacet::EType CGlobalProbeLightFacet::GetType() const
    {
        return m_Type;
    }

    // -----------------------------------------------------------------------------

    void CGlobalProbeLightFacet::SetQuality(EQuality _Quality)
    {
        m_Quality = _Quality;
    }

    // -----------------------------------------------------------------------------

    CGlobalProbeLightFacet::EQuality CGlobalProbeLightFacet::GetQuality() const
    {
        return m_Quality;
    }

    // -----------------------------------------------------------------------------

    unsigned int CGlobalProbeLightFacet::GetQualityInPixel() const
    {
        static unsigned int s_QualityInPixel[s_NumberOfQualities] = { 128, 256, 512, 1024, 2048 };

        return s_QualityInPixel[m_Quality];
    }

    // -----------------------------------------------------------------------------

    void CGlobalProbeLightFacet::SetCubemap(Dt::CTextureCube* _pCubemap)
    {
        m_pCubemap = _pCubemap;
    }

    // -----------------------------------------------------------------------------

    Dt::CTextureCube* CGlobalProbeLightFacet::GetCubemap()
    {
        return m_pCubemap;
    }

    // -----------------------------------------------------------------------------

    void CGlobalProbeLightFacet::SetIntensity(float _Intensity)
    {
        m_Intensity = _Intensity;
    }

    // -----------------------------------------------------------------------------

    float CGlobalProbeLightFacet::GetIntensity()
    {
        return m_Intensity;
    }
} // namespace Dt

namespace Dt
{
    CSkyboxFacet::CSkyboxFacet()
        : m_Type      (Procedural)
        , m_HasHDR    (true)
        , m_pCubemap  (0)
        , m_pTexture2D(0)
        , m_Intensity (0.0f)
    {

    }

    // -----------------------------------------------------------------------------

    CSkyboxFacet::~CSkyboxFacet()
    {

    }

    // -----------------------------------------------------------------------------

    void CSkyboxFacet::SetType(EType _Type)
    {
        m_Type = _Type;
    }

    // -----------------------------------------------------------------------------

    CSkyboxFacet::EType CSkyboxFacet::GetType() const
    {
        return m_Type;
    }

    // -----------------------------------------------------------------------------

    void CSkyboxFacet::SetCubemap(Dt::CTextureCube* _rCubemap)
    {
        m_pCubemap = _rCubemap;
    }

    // -----------------------------------------------------------------------------

    Dt::CTextureCube* CSkyboxFacet::GetCubemap()
    {
        return m_pCubemap;
    }

    // -----------------------------------------------------------------------------

    void CSkyboxFacet::SetTexture(Dt::CTexture2D* _pTexture2D)
    {
        m_pTexture2D = _pTexture2D;
    }

    // -----------------------------------------------------------------------------

    Dt::CTexture2D* CSkyboxFacet::GetTexture()
    {
        return m_pTexture2D;
    }

    // -----------------------------------------------------------------------------

    bool CSkyboxFacet::GetHasCubemap() const
    {
        return m_pCubemap != 0;
    }

    // -----------------------------------------------------------------------------

    bool CSkyboxFacet::GetHasTexture() const
    {
        return m_pTexture2D != 0;
    }

    // -----------------------------------------------------------------------------

    void CSkyboxFacet::SetIntensity(float _Intensity)
    {
        m_Intensity = _Intensity;
    }

    // -----------------------------------------------------------------------------

    float CSkyboxFacet::GetIntensity()
    {
        return m_Intensity;
    }
} // namespace Dt