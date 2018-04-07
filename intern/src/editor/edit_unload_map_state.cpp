
#include "editor/edit_precompiled.h"

#include "base/base_serialize_text_writer.h"

#include "core/core_console.h"

#include "data/data_entity_manager.h"
#include "data/data_map.h"

#include "editor/edit_unload_map_state.h"

#include <assert.h>
#include <fstream>
#include <string>


namespace Edit
{
    CUnloadMapState& CUnloadMapState::GetInstance()
    {
        static CUnloadMapState s_Singleton;
        
        return s_Singleton;
    }
} // namespace Edit

namespace Edit
{
    CUnloadMapState::CUnloadMapState()
        : m_NextState(CState::UnloadMap)
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    CUnloadMapState::~CUnloadMapState()
    {
        
    }

    // -----------------------------------------------------------------------------

    void CUnloadMapState::SetNextState(CState::EStateType _NextState)
    {
        m_NextState = _NextState;
    }
    
    // -----------------------------------------------------------------------------
    
    CState::EStateType CUnloadMapState::InternOnEnter()
    {
        Dt::Map          ::FreeMap();
        Dt::EntityManager::Clear();

        return Edit::CState::UnloadMap;
    }
    
    // -----------------------------------------------------------------------------
    
    CState::EStateType CUnloadMapState::InternOnLeave()
    {
        return Edit::CState::UnloadMap;
    }
    
    // -----------------------------------------------------------------------------
    
    CState::EStateType CUnloadMapState::InternOnRun()
    {
        return m_NextState;
    }
} // namespace Edit