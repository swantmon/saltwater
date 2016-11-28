
#include "data/data_precompiled.h"

#include "data/data_sky_facet.h"

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