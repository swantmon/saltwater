
#include "data/data_precompiled.h"

#include "data/data_component_manager.h"

namespace Dt
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

    void CComponentManager::RegisterDirtyComponentHandler(CComponentDelegate _NewDelegate)
    {
        m_ComponentDelegates.push_back(_NewDelegate);
    }
} // namespace Dt