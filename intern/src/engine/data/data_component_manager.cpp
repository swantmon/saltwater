
#include "engine/engine_precompiled.h"

#include "base/base_exception.h"

#include "engine/data/data_component_manager.h"
#include "engine/data/data_script_component.h"

namespace Dt
{
    CComponentManager::CComponentManager()
        : m_Components      ( )
        , m_ComponentByID   ( )
        , m_ComponentsByType( )
        , m_CurrentID       (0)
    {

    }

    // -----------------------------------------------------------------------------
    
    CComponentManager::~CComponentManager()
    {
        Clear();
    }

    // -----------------------------------------------------------------------------

    CComponentManager& CComponentManager::GetInstance()
    {
        static CComponentManager s_Instance;
        return s_Instance;
    }

    // -----------------------------------------------------------------------------

    void CComponentManager::Deallocate(Dt::IComponent* _pObject)
    {
        if (_pObject == nullptr) return;

        Deallocate(_pObject->GetID());
    }

    // -----------------------------------------------------------------------------

    void CComponentManager::Deallocate(Base::ID _ID)
    {
        auto Component = m_ComponentByID[_ID];

        if (Component == nullptr) return;

        // -----------------------------------------------------------------------------
        // Mark component as dirty
        // -----------------------------------------------------------------------------
        MarkComponentAsDirty(*Component, Dt::IComponent::DirtyDestroy);

        // -----------------------------------------------------------------------------
        // Release from organizer
        // -----------------------------------------------------------------------------
        m_ComponentByID.erase(_ID);

        auto& rComponentTypeVector = m_ComponentsByType[Component->GetTypeID()];

        auto ComponentTypeVectorIter = std::find(rComponentTypeVector.begin(), rComponentTypeVector.end(), Component);

        if (ComponentTypeVectorIter == rComponentTypeVector.end()) return;

        rComponentTypeVector.erase(ComponentTypeVectorIter);

        // -----------------------------------------------------------------------------
        // Remove unique_ptr
        // -----------------------------------------------------------------------------
        auto ComponentIter = std::find_if(m_Components.begin(), m_Components.end(), [&](std::unique_ptr<IComponent>& _rObject) { return _rObject->GetID() == _ID; });

        if (ComponentIter == m_Components.end()) return;

        m_Components.erase(ComponentIter);
    }

    // -----------------------------------------------------------------------------

    CComponentManager::CComponentDelegate::HandleType CComponentManager::RegisterDirtyComponentHandler(CComponentDelegate::FunctionType _NewDelegate)
    {
        return m_ComponentDelegate.Register(_NewDelegate);
    }

    // -----------------------------------------------------------------------------

    void CComponentManager::Clear()
    {
        for (auto& rComponent : m_Components)
        {
            MarkComponentAsDirty(*rComponent, Dt::IComponent::DirtyDestroy);
        }

        m_ComponentByID.clear();
        m_ComponentsByType.clear();
        m_Components.clear();
    }

    // -----------------------------------------------------------------------------

    void CComponentManager::Read(Base::CTextReader& _rCodec)
    {
        Base::BHash Hash = 0;
        size_t NumberOfComponents = 0;

        _rCodec >> NumberOfComponents;

        for (auto i = 0; i < NumberOfComponents; ++i)
        {
            // -----------------------------------------------------------------------------
            // Read from reader
            // -----------------------------------------------------------------------------
            _rCodec >> Hash;

            auto pNewComponent = InternAllocateByHash(Hash);

            assert(pNewComponent);

            _rCodec >> *pNewComponent;

            // -----------------------------------------------------------------------------
            // Identify ID
            // -----------------------------------------------------------------------------
            m_CurrentID = glm::max(m_CurrentID, pNewComponent->m_ID + 1);

            // -----------------------------------------------------------------------------
            // Save component to organizer
            // -----------------------------------------------------------------------------
            m_ComponentByID[pNewComponent->m_ID] = pNewComponent;

            m_ComponentsByType[pNewComponent->GetTypeID()].emplace_back(pNewComponent);
        }
    }

    // -----------------------------------------------------------------------------

    void CComponentManager::Write(Base::CTextWriter& _rCodec)
    {
        _rCodec << m_Components.size();

        for (auto& Component : m_Components)
        {
            Base::ID ID = Component->GetTypeID();

            if (ID == CScriptComponent::STATIC_TYPE_ID)
            {
                auto ScriptComponent = static_cast<CScriptComponent*>(&*Component);

                ID = ScriptComponent->GetScriptTypeID();
            }

            if (m_FactoryHash.find(ID) == std::end(m_FactoryHash))
            {
                BASE_THROWV("Failed writing component %s because hash is missing in factory.", Base::CTypeInfo::GetTypeName(Component));
            }

            Base::BHash Hash = m_FactoryHash.find(ID)->second;

            _rCodec << Hash;

            _rCodec << *Component;
        }
    }

    // -----------------------------------------------------------------------------

    void CComponentManager::MarkComponentAsDirty(IComponent& _rComponent, unsigned int _DirtyFlags)
    {
        _rComponent.m_DirtyFlags = _DirtyFlags;

        m_ComponentDelegate.Notify(&_rComponent);

        _rComponent.m_DirtyFlags = 0;
    }

    // -----------------------------------------------------------------------------

    IComponent* CComponentManager::InternAllocateByHash(Base::BHash _Hash)
    {
        assert(m_Factory.find(_Hash) != std::end(m_Factory));

        auto AllocatedComponent = m_Factory.find(_Hash)->second->Allocate();

        m_Components.emplace_back(std::unique_ptr<IComponent>(AllocatedComponent));

        return static_cast<IComponent*>(m_Components.back().get());
    }
} // namespace Dt