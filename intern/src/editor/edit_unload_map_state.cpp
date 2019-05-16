
#include "editor/edit_precompiled.h"

#include "editor/edit_unload_map_state.h"

#include "engine/data/data_entity_manager.h"
#include "engine/data/data_map.h"

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
    {
        m_NextState = CState::Exit;
    }
    
    // -----------------------------------------------------------------------------
    
    CUnloadMapState::~CUnloadMapState()
    {

    }

    // -----------------------------------------------------------------------------

    void CUnloadMapState::SaveToFile(const std::string& _rFilename)
    {
        m_Filename = _rFilename;
    }

    // -----------------------------------------------------------------------------
    
    CState::EStateType CUnloadMapState::InternOnEnter()
    {
        if (m_NextState == CState::Exit)
        {
            Dt::Map::FreeMap();
            Dt::EntityManager::Clear();
        }

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