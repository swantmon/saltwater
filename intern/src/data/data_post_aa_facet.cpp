
#include "data/data_precompiled.h"

#include "data/data_post_aa_facet.h"

namespace Dt
{
    CPostAAFXFacet::CPostAAFXFacet()
        : m_Type(SMAA)
    {
    }

    // -----------------------------------------------------------------------------

    CPostAAFXFacet::~CPostAAFXFacet()
    {
    }

    // -----------------------------------------------------------------------------

    void CPostAAFXFacet::SetType(EType _Type)
    {
        m_Type = _Type;
    }

    // -----------------------------------------------------------------------------

    CPostAAFXFacet::EType CPostAAFXFacet::GetType()
    {
        return m_Type;
    }

    // -----------------------------------------------------------------------------

    void CPostAAFXFacet::UpdateEffect()
    {
    }
} // namespace Dt