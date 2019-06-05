
#include "app_droid/app_application.h"
#include "app_droid/app_unload_map_state.h"

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