
#include "app_droid/app_application.h"
#include "app_droid/app_unload_map_state.h"

#include "base/base_serialize_text_writer.h"

#include "engine/core/core_asset_manager.h"
#include "engine/core/core_program_parameters.h"

#include "engine/data/data_entity_manager.h"
#include "engine/data/data_map.h"

namespace App
{
    CUnloadMapState& CUnloadMapState::GetInstance()
    {
        static CUnloadMapState s_Singleton;

        return s_Singleton;
    }
} // namespace App

namespace App
{
    CUnloadMapState::CUnloadMapState()
    {

    }

    // -----------------------------------------------------------------------------

    CUnloadMapState::~CUnloadMapState()
    {

    }

    // -----------------------------------------------------------------------------

    void CUnloadMapState::InternOnEnter()
    {
        // -----------------------------------------------------------------------------
        // Get filename
        // -----------------------------------------------------------------------------
        std::string Filename = Core::CProgramParameters::GetInstance().Get("application:last_scene", "Default Scene.sws");
        bool SaveScene = Core::CProgramParameters::GetInstance().Get("application:save_scene", false);

        // -----------------------------------------------------------------------------
        // Save
        // -----------------------------------------------------------------------------
        std::ofstream oStream;

        oStream.open(Core::AssetManager::GetPathToAssets() + "/" + Filename);

        if (oStream.is_open() && SaveScene)
        {
            Base::CTextWriter Writer(oStream, 1);

            Dt::CComponentManager::GetInstance().Write(Writer);
            Dt::Map::Write(Writer);
            Dt::CEntityManager::GetInstance().Write(Writer);

            oStream.close();
        }

        Dt::Map::FreeMap();
        Dt::CEntityManager::GetInstance().Clear();
        Dt::CComponentManager::GetInstance().Clear();
    }

    // -----------------------------------------------------------------------------

    void CUnloadMapState::InternOnLeave()
    {
    }

    // -----------------------------------------------------------------------------

    void CUnloadMapState::InternOnRun()
    {
        App::Application::ChangeState(App::CState::Exit);
    }
} // namespace App