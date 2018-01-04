
#include "graphic/gfx_precompiled.h"

#include "graphic/gfx_sky_facet.h"

namespace Gfx
{
    CSkyFacet::CSkyFacet()
        : m_CubemapPtr   ()
        , m_TimeStamp    (static_cast<Base::U64>(-1))
    {

    }

    // -----------------------------------------------------------------------------

    CSkyFacet::~CSkyFacet()
    {
        m_CubemapPtr = 0;
    }

    // -----------------------------------------------------------------------------

    CTexturePtr CSkyFacet::GetCubemapPtr() const
    {
        return m_CubemapPtr;
    }

    // -----------------------------------------------------------------------------

    Base::U64 CSkyFacet::GetTimeStamp() const
    {
        return m_TimeStamp;
    }
} // namespace Gfx