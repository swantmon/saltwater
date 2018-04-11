
#include "engine/engine_precompiled.h"

#include "engine/data/data_component.h"
#include "engine/data/data_component_facet.h"
#include "engine/data/data_component_manager.h"

namespace Dt
{
    CComponentFacet::CComponentFacet()
    {

    }

    // -----------------------------------------------------------------------------

    CComponentFacet::~CComponentFacet()
    {
        for (auto Component : m_Components)
        {
            Dt::CComponentManager::GetInstance().MarkComponentAsDirty(*Component, Dt::IComponent::DirtyDestroy);

            Dt::CComponentManager::GetInstance().Deallocate(Component->GetID());
        }

        m_Components.clear();
    }

    // -----------------------------------------------------------------------------

    const CComponentFacet::CComponentVector& CComponentFacet::GetComponents() const
    {
        return m_Components;
    }
} // namespace Dt