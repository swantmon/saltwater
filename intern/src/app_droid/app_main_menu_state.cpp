
#include "app_droid/app_main_menu_state.h"

namespace App
{
    CMainMenuState& CMainMenuState::GetInstance()
    {
        static CMainMenuState s_Singleton;

        return s_Singleton;
    }
} // namespace App

namespace App
{
    CMainMenuState::CMainMenuState()
    {

    }

    // -----------------------------------------------------------------------------

    CMainMenuState::~CMainMenuState()
    {

    }

    // -----------------------------------------------------------------------------

    CState::EStateType CMainMenuState::InternOnEnter()
    {
        return App::CState::MainMenu;
    }

    // -----------------------------------------------------------------------------

    CState::EStateType CMainMenuState::InternOnLeave()
    {
        return App::CState::MainMenu;
    }

    // -----------------------------------------------------------------------------

    CState::EStateType CMainMenuState::InternOnRun()
    {
        CState::EStateType NextState = CState::MainMenu;

        return NextState;
    }
} // namespace App