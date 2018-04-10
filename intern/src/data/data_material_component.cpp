
#include "engine/engine_precompiled.h"

#include "data/data_material_component.h"

namespace Dt
{
    CMaterialComponent::CMaterialComponent()
        : m_pMaterial(0)
    {
    }

    // -----------------------------------------------------------------------------

    CMaterialComponent::~CMaterialComponent()
    {

    }

    // -----------------------------------------------------------------------------

    void CMaterialComponent::SetMaterial(CMaterial* _pValue)
    {
        m_pMaterial = _pValue;
    }

    // -----------------------------------------------------------------------------

    CMaterial* CMaterialComponent::GetMaterial() const
    {
        return m_pMaterial;
    }
} // namespace Dt