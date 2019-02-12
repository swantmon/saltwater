
#include "engine/engine_precompiled.h"

#include "engine/data/data_component_manager.h"

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
        m_ComponentByID.clear();
        m_ComponentsByType.clear();
        m_Components.clear();
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
        // -----------------------------------------------------------------------------
        // Release from organizer
        // -----------------------------------------------------------------------------
        auto Component = m_ComponentByID[_ID];

        if (Component == nullptr) return;

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
        return CComponentDelegate::Register(_NewDelegate);
    }

    // -----------------------------------------------------------------------------

    void CComponentManager::MarkComponentAsDirty(IComponent& _rComponent, unsigned int _DirtyFlags)
    {
        _rComponent.m_DirtyFlags = _DirtyFlags;

        CComponentDelegate::Notify(&_rComponent);
    }
} // namespace Dt