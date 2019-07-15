
#include "engine/engine_precompiled.h"

#include "engine/data/data_ssr_component.h"

namespace Dt
{
    REGISTER_COMPONENT_SER(CSSRComponent);

    CSSRComponent::CSSRComponent()
        : m_Intensity    (1.0f)
        , m_RoughnessMask(-6.66f)
        , m_Distance     (1.0f)
        , m_UseLastFrame (true)
    {
    }

    // -----------------------------------------------------------------------------

    CSSRComponent::~CSSRComponent()
    {
    }

    // -----------------------------------------------------------------------------

    void CSSRComponent::SetIntensity(float _Intensity)
    {
        m_Intensity = _Intensity;
    }

    // -----------------------------------------------------------------------------

    float CSSRComponent::GetIntensity() const
    {
        return m_Intensity;
    }

    // -----------------------------------------------------------------------------

    void CSSRComponent::SetRoughnessMask(float _RoughnessMask)
    {
        m_RoughnessMask = _RoughnessMask;
    }

    // -----------------------------------------------------------------------------

    float CSSRComponent::GetRoughnessMask() const 
    {
        return m_RoughnessMask;
    }

    // -----------------------------------------------------------------------------

    void CSSRComponent::SetDistance(float _Distance)
    {
        m_Distance = _Distance;
    }

    // -----------------------------------------------------------------------------

    float CSSRComponent::GetDistance() const
    {
        return m_Distance;
    }

    // -----------------------------------------------------------------------------

    void CSSRComponent::SetUseLastFrame(bool _Flag)
    {
        m_UseLastFrame = _Flag;
    }

    // -----------------------------------------------------------------------------

    bool CSSRComponent::GetUseLastFrame() const
    {
        return m_UseLastFrame;
    }

    // -----------------------------------------------------------------------------

    void CSSRComponent::UpdateEffect()
    {
    }
} // namespace Dt