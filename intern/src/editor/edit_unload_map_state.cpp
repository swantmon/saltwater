
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
        : CState(UnloadMap)
    {
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
    
    void CUnloadMapState::InternOnEnter()
    {
        // -----------------------------------------------------------------------------
        // Save
        // -----------------------------------------------------------------------------
        std::ofstream oStream;

        oStream.open(m_Filename);

        Base::CTextWriter Writer(oStream, 1);

        Dt::CComponentManager::GetInstance().Write(Writer);
        Dt::Map::Write(Writer);
        Dt::CEntityManager::GetInstance().Write(Writer);

        oStream.close();

        // -----------------------------------------------------------------------------
        // Unload?
        // -----------------------------------------------------------------------------
        if (m_NextState == CState::Exit || m_NextState == CState::LoadMap)
        {
            Dt::Map::FreeMap();
            Dt::CEntityManager::GetInstance().Clear();
            Dt::CComponentManager::GetInstance().Clear();
        }
    }
    
    // -----------------------------------------------------------------------------
    
    void CUnloadMapState::InternOnLeave()
    {
        m_NextState = CState::UnloadMap;
    }
    
    // -----------------------------------------------------------------------------
    
    CState::EStateType CUnloadMapState::InternOnRun()
    {
        return m_NextState;
    }
} // namespace Edit