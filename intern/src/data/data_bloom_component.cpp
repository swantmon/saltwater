
#include "data/data_precompiled.h"

#include "base/base_memory.h"

#include "data/data_bloom_component.h"

namespace Dt
{
    CBloomComponent::CBloomComponent()
        : m_Tint         (glm::vec4(1.0f, 1.0f, 1.0f, 1.0f))
        , m_Intensity    (1.0f)
        , m_Treshhold    (1.0f)
        , m_ExposureScale(2.0f)
        , m_Size         (3)
    {
    }

    // -----------------------------------------------------------------------------

    CBloomComponent::~CBloomComponent()
    {
    }

    // -----------------------------------------------------------------------------

    void CBloomComponent::SetTint(glm::vec4& _rTint)
    {
        m_Tint = _rTint;
    }

    // -----------------------------------------------------------------------------

    glm::vec4& CBloomComponent::GetTint()
    {
        return m_Tint;
    }

    // -----------------------------------------------------------------------------

    void CBloomComponent::SetIntensity(float _Intensity)
    {
        m_Intensity = glm::clamp(_Intensity, 0.0f, 8.0f);
    }

    // -----------------------------------------------------------------------------

    float CBloomComponent::GetIntensity()
    {
        return m_Intensity;
    }

    // -----------------------------------------------------------------------------

    void CBloomComponent::SetTreshhold(float _Treshhold)
    {
        m_Treshhold = glm::clamp(_Treshhold, -1.0f, 8.0f);
    }

    // -----------------------------------------------------------------------------

    float CBloomComponent::GetTreshhold()
    {
        return m_Treshhold;
    }

    // -----------------------------------------------------------------------------

    void CBloomComponent::SetExposureScale(float _ExposureScale)
    {
        m_ExposureScale = _ExposureScale;
    }

    // -----------------------------------------------------------------------------

    float CBloomComponent::GetExposureScale()
    {
        return m_ExposureScale;
    }

    // -----------------------------------------------------------------------------

    void CBloomComponent::SetSize(unsigned int _Size)
    {
        m_Size = glm::clamp(_Size, 1u, 5u);
    }

    // -----------------------------------------------------------------------------

    unsigned int CBloomComponent::GetSize()
    {
        return m_Size;
    }

    // -----------------------------------------------------------------------------

    void CBloomComponent::UpdateEffect()
    {
    }
} // namespace Dt