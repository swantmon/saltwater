
#include "graphic/gfx_precompiled.h"

#include "graphic/gfx_light_probe_facet.h"

namespace Gfx
{
    CLightProbeFacet::CLightProbeFacet()
        : m_DiffusePtr ()
        , m_SpecularPtr()
        , m_DepthPtr   ()
        , m_TimeStamp  (static_cast<Base::U64>(-1))
    {

    }

    // -----------------------------------------------------------------------------

    CLightProbeFacet::~CLightProbeFacet()
    {
        m_DiffusePtr  = 0;
        m_SpecularPtr = 0;
        m_DepthPtr    = 0;
    }

    // -----------------------------------------------------------------------------

    CTexturePtr CLightProbeFacet::GetDiffusePtr() const
    {
        return m_DiffusePtr;
    }

    // -----------------------------------------------------------------------------

    CTexturePtr CLightProbeFacet::GetSpecularPtr() const
    {
        return m_SpecularPtr;
    }

    // -----------------------------------------------------------------------------

    CTexturePtr CLightProbeFacet::GetDepthPtr() const
    {
        return m_DepthPtr;
    }

    // -----------------------------------------------------------------------------

    Base::U64 CLightProbeFacet::GetTimeStamp() const
    {
        return m_TimeStamp;
    }
} // namespace Gfx