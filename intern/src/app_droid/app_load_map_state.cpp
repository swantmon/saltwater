
#include "app_droid/app_load_map_state.h"

namespace App
{
    CLoadMapState& CLoadMapState::GetInstance()
    {
        static CLoadMapState s_Singleton;

        return s_Singleton;
    }
} // namespace App

namespace App
{
    CLoadMapState::CLoadMapState()
    {

    }

    // -----------------------------------------------------------------------------

    CLoadMapState::~CLoadMapState()
    {

    }

    // -----------------------------------------------------------------------------

    CState::EStateType CLoadMapState::InternOnEnter()
    {
        return App::CState::LoadMap;
    }

    // -----------------------------------------------------------------------------

    CState::EStateType CLoadMapState::InternOnLeave()
    {
        return App::CState::LoadMap;
    }

    // -----------------------------------------------------------------------------

    CState::EStateType CLoadMapState::InternOnRun()
    {
        CState::EStateType NextState = CState::Play;

        return NextState;
    }
} // namespace App