
#include "app_droid/app_unload_map_state.h"

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

    CState::EStateType CUnloadMapState::InternOnEnter()
    {
        return App::CState::UnloadMap;
    }

    // -----------------------------------------------------------------------------

    CState::EStateType CUnloadMapState::InternOnLeave()
    {
        return App::CState::UnloadMap;
    }

    // -----------------------------------------------------------------------------

    CState::EStateType CUnloadMapState::InternOnRun()
    {
        CState::EStateType NextState = CState::Exit;

        return NextState;
    }
} // namespace App