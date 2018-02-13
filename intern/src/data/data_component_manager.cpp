
#include "data/data_precompiled.h"

#include "data/data_component_manager.h"

namespace Dt
{
    CComponentManager::CComponentManager()
        : m_Components        ( )
        , m_ComponentDelegates( )
        , m_CurrentID         (0)
    {

    }

    // -----------------------------------------------------------------------------
    
    CComponentManager::~CComponentManager()
    {

    }

    // -----------------------------------------------------------------------------


    void CComponentManager::OnStart()
    {

    }

    // -----------------------------------------------------------------------------

    void CComponentManager::OnExit()
    {
        for (auto Element : m_Components)
        {
            Base::CMemory::DeleteObject(Element);
        }

        m_Components.clear();
    }

    // -----------------------------------------------------------------------------

    void CComponentManager::Update()
    {

    }

    // -----------------------------------------------------------------------------

    void CComponentManager::MarkComponentAsDirty(Dt::IComponent& _rComponent, unsigned int _DirtyFlags)
    {
        _rComponent.SetDirtyFlags(_DirtyFlags);

        for (auto Delegate : m_ComponentDelegates)
        {
            Delegate(_rComponent.GetTypeID(), &_rComponent);
        }
    }

    // -----------------------------------------------------------------------------

    void CComponentManager::RegisterDirtyComponentHandler(CComponentDelegate _NewDelegate)
    {
        m_ComponentDelegates.push_back(_NewDelegate);
    }
} // namespace Dt