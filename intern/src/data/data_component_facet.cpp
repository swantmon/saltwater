
#include "data/data_precompiled.h"

#include "data/data_component_facet.h"

namespace Dt
{
    CComponentFacet::CComponentFacet()
    {

    }

    // -----------------------------------------------------------------------------

    CComponentFacet::~CComponentFacet()
    {

    }

    // -----------------------------------------------------------------------------

    const CComponentFacet::CComponentVector& CComponentFacet::GetComponents() const
    {
        return m_Components;
    }
} // namespace Dt