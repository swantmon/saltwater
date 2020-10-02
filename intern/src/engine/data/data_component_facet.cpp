
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

    CComponentFacet::CComponentVector& CComponentFacet::GetComponents()
    {
        return m_Components;
    }

    // -----------------------------------------------------------------------------

    const CComponentFacet::CComponentVector& CComponentFacet::GetComponents() const
    {
        return m_Components;
    }

    // -----------------------------------------------------------------------------

    void CComponentFacet::Read(CSceneReader& _rCodec)
    {
        size_t NumberOfComponents;

        _rCodec >> NumberOfComponents;

        m_Components.resize(NumberOfComponents);

        for (auto i = 0; i < NumberOfComponents; ++i)
        {
            Base::ID ID;

            _rCodec >> ID;

            m_Components[i] = Dt::CComponentManager::GetInstance().GetComponent<Dt::IComponent>(ID);
        }
    }

    // -----------------------------------------------------------------------------

    void CComponentFacet::Write(CSceneWriter& _rCodec)
    {
        auto NumberOfComponents = m_Components.size();

        _rCodec << NumberOfComponents;

        for (auto Component : m_Components)
        {
            assert(Component != nullptr);

            _rCodec << Component->GetID();
        }
    }
} // namespace Dt