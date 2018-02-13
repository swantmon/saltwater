
#include "data/data_precompiled.h"

#include "data/data_sky_component.h"

namespace Dt
{
    CSkyComponent::CSkyComponent()
        : m_RefreshMode     (Static)
        , m_Type            (Procedural)
        , m_HasHDR          (true)
        , m_pCubemap        (0)
        , m_pPanoramaTexture(0)
        , m_pTexture        (0)
        , m_Intensity       (0.0f)
    {

    }

    // -----------------------------------------------------------------------------

    CSkyComponent::~CSkyComponent()
    {

    }

    // -----------------------------------------------------------------------------

    void CSkyComponent::SetRefreshMode(ERefreshMode _RefreshMode)
    {
        m_RefreshMode = _RefreshMode;
    }

    // -----------------------------------------------------------------------------

    CSkyComponent::ERefreshMode CSkyComponent::GetRefreshMode()
    {
        return m_RefreshMode;
    }

    // -----------------------------------------------------------------------------

    void CSkyComponent::SetType(EType _Type)
    {
        m_Type = _Type;
    }

    // -----------------------------------------------------------------------------

    CSkyComponent::EType CSkyComponent::GetType() const
    {
        return m_Type;
    }

    // -----------------------------------------------------------------------------

    void CSkyComponent::SetCubemap(Dt::CTextureCube* _rCubemap)
    {
        m_pCubemap = _rCubemap;
    }

    // -----------------------------------------------------------------------------

    Dt::CTextureCube* CSkyComponent::GetCubemap()
    {
        return m_pCubemap;
    }

    // -----------------------------------------------------------------------------

    void CSkyComponent::SetPanorama(Dt::CTexture2D* _pTexture2D)
    {
        m_pPanoramaTexture = _pTexture2D;
    }

    // -----------------------------------------------------------------------------

    Dt::CTexture2D* CSkyComponent::GetPanorama()
    {
        return m_pPanoramaTexture;
    }

    // -----------------------------------------------------------------------------

    void CSkyComponent::SetTexture(Dt::CTexture2D* _pTexture2D)
    {
        m_pTexture = _pTexture2D;
    }

    // -----------------------------------------------------------------------------

    Dt::CTexture2D* CSkyComponent::GetTexture()
    {
        return m_pTexture;
    }

    // -----------------------------------------------------------------------------

    bool CSkyComponent::GetHasCubemap() const
    {
        return m_pCubemap != 0;
    }

    // -----------------------------------------------------------------------------

    bool CSkyComponent::GetHasPanorama() const
    {
        return m_pPanoramaTexture != 0;
    }

    // -----------------------------------------------------------------------------

    bool CSkyComponent::GetHasTexture() const
    {
        return m_pTexture != 0;
    }

    // -----------------------------------------------------------------------------

    void CSkyComponent::SetIntensity(float _Intensity)
    {
        m_Intensity = _Intensity;
    }

    // -----------------------------------------------------------------------------

    float CSkyComponent::GetIntensity() const
    {
        return m_Intensity;
    }
} // namespace Dt