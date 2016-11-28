
#include "data/data_precompiled.h"

#include "data/data_fxaa_facet.h"

namespace Dt
{
    CFXAAFXFacet::CFXAAFXFacet()
        : m_Luma(Base::Float3(0.299f, 0.587f, 0.144f))
    {
    }

    // -----------------------------------------------------------------------------

    CFXAAFXFacet::~CFXAAFXFacet()
    {
    }

    // -----------------------------------------------------------------------------

    void CFXAAFXFacet::SetLuma(Base::Float3& _rLuma)
    {
        m_Luma = _rLuma;
    }

    // -----------------------------------------------------------------------------

    Base::Float3& CFXAAFXFacet::GetLuma()
    {
        return m_Luma;
    }

    // -----------------------------------------------------------------------------

    void CFXAAFXFacet::UpdateEffect()
    {
    }
} // namespace Dt