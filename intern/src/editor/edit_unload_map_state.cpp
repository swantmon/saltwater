
#include "editor/edit_precompiled.h"

#include "editor/edit_unload_map_state.h"

#include "engine/core/core_asset_manager.h"
#include "engine/core/core_program_parameters.h"

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
        : CState         (UnloadMap)
        , m_Filename     ("Default Scene.sws")
        , m_PreventSaving(true)
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

    const std::string& CUnloadMapState::GetFilename() const
    {
        return m_Filename;
    }

    // -----------------------------------------------------------------------------
    
    void CUnloadMapState::PreventSaving(bool _Flag)
    {
        m_PreventSaving = _Flag;
    }

    // -----------------------------------------------------------------------------
    
    void CUnloadMapState::InternOnEnter()
    {
        // -----------------------------------------------------------------------------
        // Save
        // -----------------------------------------------------------------------------
        if (!m_PreventSaving)
        {
            std::ofstream oStream;

            oStream.open(Core::AssetManager::GetPathToAssets() + "/" + m_Filename);

            if (oStream.is_open())
            {
                Base::CTextWriter Writer(oStream, 1);

                Dt::CComponentManager::GetInstance().Write(Writer);
                Dt::Map::Write(Writer);
                Dt::CEntityManager::GetInstance().Write(Writer);

                oStream.close();

                Core::CProgramParameters::GetInstance().Set("application:last_scene", m_Filename);

                ENGINE_CONSOLE_INFOV("Scene '%s' has been saved succesfully.", m_Filename.c_str());
            }
            else
            {
                ENGINE_CONSOLE_ERROR("Scene cannot be saved because the file could not be created or is already in use. Maybe the folder is missing?");
            }
        }
        else
        {
            ENGINE_CONSOLE_INFOV("Scene '%s' has not been saved.", m_Filename.c_str());
        }

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

        m_PreventSaving = true;
    }
    
    // -----------------------------------------------------------------------------
    
    CState::EStateType CUnloadMapState::InternOnRun()
    {
        return m_NextState;
    }
} // namespace Edit