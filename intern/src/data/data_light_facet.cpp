
#include "data/data_precompiled.h"

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
    CLightProbeFacet::CLightProbeFacet()
        : m_RefreshMode(Static)
        , m_Type       (Sky)
        , m_Quality    (PX128)
        , m_pCubemap   ()
        , m_Intensity  (0)
    {

    }

    // -----------------------------------------------------------------------------

    CLightProbeFacet::~CLightProbeFacet()
    {

    }

    // -----------------------------------------------------------------------------

    void CLightProbeFacet::SetRefreshMode(ERefreshMode _RefreshMode)
    {
        m_RefreshMode = _RefreshMode;
    }

    // -----------------------------------------------------------------------------

    CLightProbeFacet::ERefreshMode CLightProbeFacet::GetRefreshMode()
    {
        return m_RefreshMode;
    }

    // -----------------------------------------------------------------------------

    void CLightProbeFacet::SetType(EType _Type)
    {
        m_Type = _Type;
    }

    // -----------------------------------------------------------------------------

    CLightProbeFacet::EType CLightProbeFacet::GetType() const
    {
        return m_Type;
    }

    // -----------------------------------------------------------------------------

    void CLightProbeFacet::SetQuality(EQuality _Quality)
    {
        m_Quality = _Quality;
    }

    // -----------------------------------------------------------------------------

    CLightProbeFacet::EQuality CLightProbeFacet::GetQuality() const
    {
        return m_Quality;
    }

    // -----------------------------------------------------------------------------

    unsigned int CLightProbeFacet::GetQualityInPixel() const
    {
        static unsigned int s_QualityInPixel[s_NumberOfQualities] = { 128, 256, 512, 1024, 2048 };

        return s_QualityInPixel[m_Quality];
    }

    // -----------------------------------------------------------------------------

    void CLightProbeFacet::SetCubemap(Dt::CTextureCube* _pCubemap)
    {
        m_pCubemap = _pCubemap;
    }

    // -----------------------------------------------------------------------------

    Dt::CTextureCube* CLightProbeFacet::GetCubemap()
    {
        return m_pCubemap;
    }

    // -----------------------------------------------------------------------------

    void CLightProbeFacet::SetIntensity(float _Intensity)
    {
        m_Intensity = _Intensity;
    }

    // -----------------------------------------------------------------------------

    float CLightProbeFacet::GetIntensity()
    {
        return m_Intensity;
    }
} // namespace Dt

namespace Dt
{
    CSkyFacet::CSkyFacet()
        : m_Type            (Procedural)
        , m_HasHDR          (true)
        , m_pCubemap        (0)
        , m_pPanoramaTexture(0)
        , m_pTexture        (0)
        , m_Intensity       (0.0f)
    {

    }

    // -----------------------------------------------------------------------------

    CSkyFacet::~CSkyFacet()
    {

    }

    // -----------------------------------------------------------------------------

    void CSkyFacet::SetType(EType _Type)
    {
        m_Type = _Type;
    }

    // -----------------------------------------------------------------------------

    CSkyFacet::EType CSkyFacet::GetType() const
    {
        return m_Type;
    }

    // -----------------------------------------------------------------------------

    void CSkyFacet::SetCubemap(Dt::CTextureCube* _rCubemap)
    {
        m_pCubemap = _rCubemap;
    }

    // -----------------------------------------------------------------------------

    Dt::CTextureCube* CSkyFacet::GetCubemap()
    {
        return m_pCubemap;
    }

    // -----------------------------------------------------------------------------

    void CSkyFacet::SetPanorama(Dt::CTexture2D* _pTexture2D)
    {
        m_pPanoramaTexture = _pTexture2D;
    }

    // -----------------------------------------------------------------------------

    Dt::CTexture2D* CSkyFacet::GetPanorama()
    {
        return m_pPanoramaTexture;
    }

    // -----------------------------------------------------------------------------

    void CSkyFacet::SetTexture(Dt::CTexture2D* _pTexture2D)
    {
        m_pTexture = _pTexture2D;
    }

    // -----------------------------------------------------------------------------

    Dt::CTexture2D* CSkyFacet::GetTexture()
    {
        return m_pTexture;
    }

    // -----------------------------------------------------------------------------

    bool CSkyFacet::GetHasCubemap() const
    {
        return m_pCubemap != 0;
    }

    // -----------------------------------------------------------------------------

    bool CSkyFacet::GetHasPanorama() const
    {
        return m_pPanoramaTexture != 0;
    }

    // -----------------------------------------------------------------------------

    bool CSkyFacet::GetHasTexture() const
    {
        return m_pTexture != 0;
    }

    // -----------------------------------------------------------------------------

    void CSkyFacet::SetIntensity(float _Intensity)
    {
        m_Intensity = _Intensity;
    }

    // -----------------------------------------------------------------------------

    float CSkyFacet::GetIntensity() const
    {
        return m_Intensity;
    }
} // namespace Dt