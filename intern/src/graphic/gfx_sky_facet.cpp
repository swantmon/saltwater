
#include "graphic/gfx_precompiled.h"

#include "graphic/gfx_sky_facet.h"

namespace Gfx
{
    CSkyFacet::CSkyFacet()
        : m_CubemapPtr   ()
        , m_CubemapSetPtr()
        , m_TimeStamp    (static_cast<Base::U64>(-1))
    {

    }

    // -----------------------------------------------------------------------------

    CSkyFacet::~CSkyFacet()
    {
        m_CubemapPtr    = 0;
        m_CubemapSetPtr = 0;
    }

    // -----------------------------------------------------------------------------

    CTexture2DPtr CSkyFacet::GetCubemapPtr() const
    {
        return m_CubemapPtr;
    }

    // -----------------------------------------------------------------------------

    CTextureSetPtr CSkyFacet::GetCubemapSetPtr() const
    {
        return m_CubemapSetPtr;
    }

    // -----------------------------------------------------------------------------

    Base::U64 CSkyFacet::GetTimeStamp() const
    {
        return m_TimeStamp;
    }
} // namespace Gfx