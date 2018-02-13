
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
        m_Components.clear();
    }

    // -----------------------------------------------------------------------------

    void CComponentManager::MarkComponentAsDirty(Dt::IComponent* _pComponent, unsigned int _DirtyFlags)
    {
        _pComponent->SetDirtyFlags(_DirtyFlags);

        for (auto Delegate : m_ComponentDelegates)
        {
            Delegate(_pComponent->GetTypeID(), _pComponent);
        }
    }

    // -----------------------------------------------------------------------------

    void CComponentManager::RegisterDirtyComponentHandler(CComponentDelegate _NewDelegate)
    {
        m_ComponentDelegates.push_back(_NewDelegate);
    }
} // namespace Dt