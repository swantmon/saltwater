
#include "data/data_precompiled.h"

#include "data/data_dof_facet.h"

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

    glm::vec2& CDOFFXFacet::GetNear()
    {
        return m_Near;
    }

    // -----------------------------------------------------------------------------

    glm::vec4& CDOFFXFacet::GetLerpScale()
    {
        return m_LerpScale;
    }

    // -----------------------------------------------------------------------------

    glm::vec4& CDOFFXFacet::GetLerpBias()
    {
        return m_LerpBias;
    }

    // -----------------------------------------------------------------------------

    glm::vec3& CDOFFXFacet::GetEqFar()
    {
        return m_EqFar;
    }

    // -----------------------------------------------------------------------------

    void CDOFFXFacet::UpdateEffect()
    {
        auto CalculateLerpDistances = [](glm::vec4& _rLerpScale, glm::vec4& _rLerpBias, float _FadeUnToSmallBlur, float _FadeSmallToMediumBlur)
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
            
            _rLerpScale = glm::vec4(-1.0f / _FadeUnToSmallBlur, -1.0f / _FadeSmallToMediumBlur                         , -1.0f / FadeMediumToLargeBlur, 1.0f / FadeMediumToLargeBlur);
            _rLerpBias  = glm::vec4( 1.0f                     , (1.0f - FadeMediumToLargeBlur) / _FadeSmallToMediumBlur,  1.0f / FadeMediumToLargeBlur, (FadeMediumToLargeBlur - 1.0f) / FadeMediumToLargeBlur);
        };
        
        // -----------------------------------------------------------------------------
        
        auto CalculateEquationFarDistance = [](glm::vec3& _rEqFar, float _StartDistance, float _NearToFarRatio)
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
        
        auto CalculateNearDistance = [](glm::vec2& _rNear, float _FinishDistance)
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