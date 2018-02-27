
#include "base/base_precompiled.h"

#include "base/base_component_manager.h"

namespace CON
{
    CComponentManager::CComponentManager()
        : m_Components        ( )
        , m_ComponentByID     ( )
        , m_ComponentsByType  ( )
        , m_ComponentDelegates( )
        , m_CurrentID         (0)
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

    void CComponentManager::Deallocate(Base::ID _ID)
    {
        // -----------------------------------------------------------------------------
        // Release from organizer
        // -----------------------------------------------------------------------------
        auto Component = m_ComponentByID[_ID];

        if (Component == nullptr) return;

        m_ComponentByID.erase(_ID);

        auto ComponentTypeVector = m_ComponentsByType[Component->GetTypeID()];

        auto ComponentTypeVectorIter = std::find(ComponentTypeVector.begin(), ComponentTypeVector.end(), Component);

        if (ComponentTypeVectorIter == ComponentTypeVector.end()) return;

        ComponentTypeVector.erase(ComponentTypeVectorIter);

        // -----------------------------------------------------------------------------
        // Remove unique_ptr
        // -----------------------------------------------------------------------------
        auto ComponentIter = std::find_if(m_Components.begin(), m_Components.end(), [&](std::unique_ptr<IComponent>& _rObject) { return _rObject->GetID() == _ID; });

        if (ComponentIter == m_Components.end()) return;

        m_Components.erase(ComponentIter);
    }

    // -----------------------------------------------------------------------------

    void CComponentManager::RegisterDirtyComponentHandler(CComponentDelegate _NewDelegate)
    {
        m_ComponentDelegates.push_back(_NewDelegate);
    }
} // namespace CON