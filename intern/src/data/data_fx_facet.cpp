
#include "data/data_precompiled.h"

#include "base/base_memory.h"

#include "data/data_fx_facet.h"

namespace Dt
{
    CBloomFXFacet::CBloomFXFacet()
        : m_Tint         (Base::Float4(1.0f, 1.0f, 1.0f, 1.0f))
        , m_Intensity    (1.0f)
        , m_Treshhold    (1.0f)
        , m_ExposureScale(2.0f)
        , m_Size         (3)
    {
    }

    // -----------------------------------------------------------------------------

    CBloomFXFacet::~CBloomFXFacet()
    {
    }

    // -----------------------------------------------------------------------------

    void CBloomFXFacet::SetTint(Base::Float4& _rTint)
    {
        m_Tint = _rTint;
    }

    // -----------------------------------------------------------------------------

    Base::Float4& CBloomFXFacet::GetTint()
    {
        return m_Tint;
    }

    // -----------------------------------------------------------------------------

    void CBloomFXFacet::SetIntensity(float _Intensity)
    {
        m_Intensity = Base::Clamp(_Intensity, 0.0f, 8.0f);
    }

    // -----------------------------------------------------------------------------

    float CBloomFXFacet::GetIntensity()
    {
        return m_Intensity;
    }

    // -----------------------------------------------------------------------------

    void CBloomFXFacet::SetTreshhold(float _Treshhold)
    {
        m_Treshhold = Base::Clamp(_Treshhold, -1.0f, 8.0f);
    }

    // -----------------------------------------------------------------------------

    float CBloomFXFacet::GetTreshhold()
    {
        return m_Treshhold;
    }

    // -----------------------------------------------------------------------------

    void CBloomFXFacet::SetExposureScale(float _ExposureScale)
    {
        m_ExposureScale = _ExposureScale;
    }

    // -----------------------------------------------------------------------------

    float CBloomFXFacet::GetExposureScale()
    {
        return m_ExposureScale;
    }

    // -----------------------------------------------------------------------------

    void CBloomFXFacet::SetSize(unsigned int _Size)
    {
        m_Size = Base::Clamp(_Size, 1u, 5u);
    }

    // -----------------------------------------------------------------------------

    unsigned int CBloomFXFacet::GetSize()
    {
        return m_Size;
    }

    // -----------------------------------------------------------------------------

    void CBloomFXFacet::UpdateEffect()
    {
    }
} // namespace Dt

namespace Dt
{
    CSSRFXFacet::CSSRFXFacet()
        : m_Intensity    (1.0f)
        , m_RoughnessMask(-6.66f)
        , m_Distance     (1.0f)
        , m_UseLastFrame (true)
    {
    }

    // -----------------------------------------------------------------------------

    CSSRFXFacet::~CSSRFXFacet()
    {
    }

    // -----------------------------------------------------------------------------

    void CSSRFXFacet::SetIntensity(float _Intensity)
    {
        m_Intensity = _Intensity;
    }

    // -----------------------------------------------------------------------------

    float CSSRFXFacet::GetIntensity() const
    {
        return m_Intensity;
    }

    // -----------------------------------------------------------------------------

    void CSSRFXFacet::SetRoughnessMask(float _RoughnessMask)
    {
        m_RoughnessMask = _RoughnessMask;
    }

    // -----------------------------------------------------------------------------

    float CSSRFXFacet::GetRoughnessMask() const 
    {
        return m_RoughnessMask;
    }

    // -----------------------------------------------------------------------------

    void CSSRFXFacet::SetDistance(float _Distance)
    {
        m_Distance = _Distance;
    }

    // -----------------------------------------------------------------------------

    float CSSRFXFacet::GetDistance() const
    {
        return m_Distance;
    }

    // -----------------------------------------------------------------------------

    void CSSRFXFacet::SetUseLastFrame(bool _Flag)
    {
        m_UseLastFrame = _Flag;
    }

    // -----------------------------------------------------------------------------

    bool CSSRFXFacet::GetUseLastFrame() const
    {
        return m_UseLastFrame;
    }

    // -----------------------------------------------------------------------------

    void CSSRFXFacet::UpdateEffect()
    {
    }
} // namespace Dt

namespace Dt
{
    CDOFFXFacet::CDOFFXFacet()
        : m_NearDistance         (0.01f)
        , m_FarDistance          (0.5f)
        , m_NearToFarRatio       (0.8f)
        , m_FadeUnToSmallBlur    (0.05f)
        , m_FadeSmallToMediumBlur(0.3f)
        , m_Near                 ()
        , m_LerpScale            ()
        , m_LerpBias             ()
        , m_EqFar                ()
    {
        UpdateEffect();
    }

    // -----------------------------------------------------------------------------

    CDOFFXFacet::~CDOFFXFacet()
    {
    }

    // -----------------------------------------------------------------------------

    void CDOFFXFacet::SetNearDistance(float _NearDistance)
    {
        m_NearDistance = _NearDistance;
    }

    // -----------------------------------------------------------------------------

    float CDOFFXFacet::GetNearDistance()
    {
        return m_NearDistance;
    }

    // -----------------------------------------------------------------------------

    void CDOFFXFacet::SetFarDistance(float _FarDistance)
    {
        m_FarDistance = _FarDistance;
    }

    // -----------------------------------------------------------------------------

    float CDOFFXFacet::GetFarDistance()
    {
        return m_FarDistance;
    }

    // -----------------------------------------------------------------------------

    void CDOFFXFacet::SetNearToFarRatio(float _NearToFarRatio)
    {
        m_NearToFarRatio = _NearToFarRatio;
    }

    // -----------------------------------------------------------------------------

    float CDOFFXFacet::GetNearToFarRatio()
    {
        return m_NearToFarRatio;
    }

    // -----------------------------------------------------------------------------

    void CDOFFXFacet::SetFadeUnToSmallBlur(float _FadeUnToSmallBlur)
    {
        m_FadeUnToSmallBlur = _FadeUnToSmallBlur;
    }

    // -----------------------------------------------------------------------------

    float CDOFFXFacet::GetFadeUnToSmallBlur()
    {
        return m_FadeUnToSmallBlur;
    }

    // -----------------------------------------------------------------------------

    void CDOFFXFacet::SetFadeSmallToMediumBlur(float _FadeSmallToMediumBlur)
    {
        m_FadeSmallToMediumBlur = _FadeSmallToMediumBlur;
    }

    // -----------------------------------------------------------------------------

    float CDOFFXFacet::GetFadeSmallToMediumBlur()
    {
        return m_FadeSmallToMediumBlur;
    }

    // -----------------------------------------------------------------------------

    Base::Float2& CDOFFXFacet::GetNear()
    {
        return m_Near;
    }

    // -----------------------------------------------------------------------------

    Base::Float4& CDOFFXFacet::GetLerpScale()
    {
        return m_LerpScale;
    }

    // -----------------------------------------------------------------------------

    Base::Float4& CDOFFXFacet::GetLerpBias()
    {
        return m_LerpBias;
    }

    // -----------------------------------------------------------------------------

    Base::Float3& CDOFFXFacet::GetEqFar()
    {
        return m_EqFar;
    }

    // -----------------------------------------------------------------------------

    void CDOFFXFacet::UpdateEffect()
    {
        auto CalculateLerpDistances = [](Base::Float4& _rLerpScale, Base::Float4& _rLerpBias, float _FadeUnToSmallBlur, float _FadeSmallToMediumBlur)
        {
            // -----------------------------------------------------------------------------
            // NOTE: D0 + D1 have to be between 0 and 1.
            //
            // Specify D0 and D1. D2 will be automatically calculated (1 - (d0 + d1)),
            // Efficiently calculate the cross-blend weights for each sample.
            // Let the unblurred sample to small blur fade happen over distance
            // d0, the small to medium blur over distance d1, and the medium to
            // large blur over distance d2, where d0 + d1 + d2 = 1.
            //
            // dofLerpScale = float4( -1 / d0,       -1 / d1, -1 / d2,        1 / d2 );
            // dofLerpBias  = float4(  1     , (1 - d2) / d1,  1 / d2, (d2 - 1) / d2 );
            //
            // d0 = Fade from un to small blurred;
            // d1 = Fade from small to medium blurred;
            // d2 = Fade from medium to large blurred;
            //
            // -----------------------------------------------------------------------------
            
            float FadeMediumToLargeBlur = 1.0f - (_FadeUnToSmallBlur + _FadeSmallToMediumBlur);
            
            _rLerpScale = Base::Float4(-1.0f / _FadeUnToSmallBlur, -1.0f / _FadeSmallToMediumBlur                         , -1.0f / FadeMediumToLargeBlur, 1.0f / FadeMediumToLargeBlur);
            _rLerpBias  = Base::Float4( 1.0f                     , (1.0f - FadeMediumToLargeBlur) / _FadeSmallToMediumBlur,  1.0f / FadeMediumToLargeBlur, (FadeMediumToLargeBlur - 1.0f) / FadeMediumToLargeBlur);
        };
        
        // -----------------------------------------------------------------------------
        
        auto CalculateEquationFarDistance = [](Base::Float3& _rEqFar, float _StartDistance, float _NearToFarRatio)
        {
            // -----------------------------------------------------------------------------
            // Set the far distance start point for the DOF
            // Example: _StartDistance = 0.5f, _NearToFarRatio = 0.8f
            // -----------------------------------------------------------------------------
            _rEqFar[0] = 1.0f / (1.0f - _StartDistance);
            _rEqFar[1] = 1.0f - _rEqFar[0];
            _rEqFar[2] = _NearToFarRatio;
        };
        
        // -----------------------------------------------------------------------------
        
        auto CalculateNearDistance = [](Base::Float2& _rNear, float _FinishDistance)
        {
            // -----------------------------------------------------------------------------
            // DofNear: (Linear distance up to everything should be blurred):
            // y = 1
            // x = -y / Far_Distance
            // -----------------------------------------------------------------------------
            _rNear[1] =  1.0f;
            _rNear[0] = -_rNear[1] / _FinishDistance;
        };

        CalculateNearDistance(m_Near, m_NearDistance);
        CalculateLerpDistances(m_LerpScale, m_LerpBias, m_FadeUnToSmallBlur, m_FadeSmallToMediumBlur);
        CalculateEquationFarDistance(m_EqFar, m_FarDistance, m_NearToFarRatio);
    }
} // namespace Dt

namespace Dt
{
    CFXAAFXFacet::CFXAAFXFacet()
        : m_Luma(Base::Float3(0.299f, 0.587f, 0.144f))
    {
    }

    // -----------------------------------------------------------------------------

    CFXAAFXFacet::~CFXAAFXFacet()
    {
    }

    // -----------------------------------------------------------------------------

    void CFXAAFXFacet::SetLuma(Base::Float3& _rLuma)
    {
        m_Luma = _rLuma;
    }

    // -----------------------------------------------------------------------------

    Base::Float3& CFXAAFXFacet::GetLuma()
    {
        return m_Luma;
    }

    // -----------------------------------------------------------------------------

    void CFXAAFXFacet::UpdateEffect()
    {
    }
} // namespace Dt

namespace Dt
{
    CSSAOFXFacet::CSSAOFXFacet()
    {
    }

    // -----------------------------------------------------------------------------

    CSSAOFXFacet::~CSSAOFXFacet()
    {
    }
} // namespace Dt

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