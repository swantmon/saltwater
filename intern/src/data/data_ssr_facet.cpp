
#include "data/data_precompiled.h"

#include "data/data_ssr_facet.h"

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