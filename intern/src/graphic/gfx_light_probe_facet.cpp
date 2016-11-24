
#include "graphic/gfx_precompiled.h"

#include "graphic/gfx_light_probe_facet.h"

namespace Gfx
{
    CLightProbeFacet::CLightProbeFacet()
        : m_DiffusePtr    ()
        , m_SpecularPtr   ()
        , m_FilteredSetPtr()
        , m_TimeStamp     (static_cast<Base::U64>(-1))
    {

    }

    // -----------------------------------------------------------------------------

    CLightProbeFacet::~CLightProbeFacet()
    {
        m_DiffusePtr     = 0;
        m_SpecularPtr    = 0;
        m_FilteredSetPtr = 0;
    }

    // -----------------------------------------------------------------------------

    CTexture2DPtr CLightProbeFacet::GetDiffusePtr() const
    {
        return m_DiffusePtr;
    }

    // -----------------------------------------------------------------------------

    CTexture2DPtr CLightProbeFacet::GetSpecularPtr() const
    {
        return m_SpecularPtr;
    }

    // -----------------------------------------------------------------------------


    CTextureSetPtr CLightProbeFacet::GetFilteredSetPtr() const
    {
        return m_FilteredSetPtr;
    }

    // -----------------------------------------------------------------------------

    Base::U64 CLightProbeFacet::GetTimeStamp() const
    {
        return m_TimeStamp;
    }
} // namespace Gfx