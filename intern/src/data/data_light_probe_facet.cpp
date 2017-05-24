
#include "data/data_precompiled.h"

#include "data/data_light_probe_facet.h"

namespace Dt
{
    CLightProbeFacet::CLightProbeFacet()
        : m_RefreshMode       (Static)
        , m_Type              (Sky)
        , m_Quality           (PX128)
        , m_pCubemap          ()
        , m_Intensity         (0)
        , m_Near              (0.1f)
        , m_Far               (10.0f)
        , m_ParallaxCorrection(false)
        , m_BoxSize           (10.0f)
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

    float CLightProbeFacet::GetIntensity() const
    {
        return m_Intensity;
    }

    // -----------------------------------------------------------------------------

    void CLightProbeFacet::SetNear(float _Near)
    {
        m_Near = _Near;
    }

    // -----------------------------------------------------------------------------

    float CLightProbeFacet::GetNear() const
    {
        return m_Near;
    }

    // -----------------------------------------------------------------------------

    void CLightProbeFacet::SetFar(float _Far)
    {
        m_Far = _Far;
    }

    // -----------------------------------------------------------------------------

    float CLightProbeFacet::GetFar() const
    {
        return m_Far;
    }

    void CLightProbeFacet::SetParallaxCorrection(bool _Flag)
    {
        m_ParallaxCorrection = _Flag;
    }

    // -----------------------------------------------------------------------------

    bool CLightProbeFacet::GetParallaxCorrection() const
    {
        return m_ParallaxCorrection;
    }

    // -----------------------------------------------------------------------------

    void CLightProbeFacet::SetBoxSize(const Base::Float3& _rSize)
    {
        m_BoxSize = _rSize;
    }

    // -----------------------------------------------------------------------------

    const Base::Float3& CLightProbeFacet::GetBoxSize() const
    {
        return m_BoxSize;
    }
} // namespace Dt