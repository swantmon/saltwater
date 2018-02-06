
#include "data/data_precompiled.h"

#include "base/base_memory.h"

#include "data/data_bloom_facet.h"

namespace Dt
{
    CBloomFXFacet::CBloomFXFacet()
        : m_Tint         (glm::vec4(1.0f, 1.0f, 1.0f, 1.0f))
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

    void CBloomFXFacet::SetTint(glm::vec4& _rTint)
    {
        m_Tint = _rTint;
    }

    // -----------------------------------------------------------------------------

    glm::vec4& CBloomFXFacet::GetTint()
    {
        return m_Tint;
    }

    // -----------------------------------------------------------------------------

    void CBloomFXFacet::SetIntensity(float _Intensity)
    {
        m_Intensity = glm::clamp(_Intensity, 0.0f, 8.0f);
    }

    // -----------------------------------------------------------------------------

    float CBloomFXFacet::GetIntensity()
    {
        return m_Intensity;
    }

    // -----------------------------------------------------------------------------

    void CBloomFXFacet::SetTreshhold(float _Treshhold)
    {
        m_Treshhold = glm::clamp(_Treshhold, -1.0f, 8.0f);
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
        m_Size = glm::clamp(_Size, 1u, 5u);
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