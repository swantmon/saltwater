
#include "app_droid/app_application.h"
#include "app_droid/app_main_menu_state.h"

#include "graphic/gfx_main_menu_state.h"

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

    void CMainMenuState::InternOnEnter()
    {
        Gfx::MainMenu::OnEnter();
    }

    // -----------------------------------------------------------------------------

    void CMainMenuState::InternOnLeave()
    {
        Gfx::MainMenu::OnLeave();
    }

    // -----------------------------------------------------------------------------

    void CMainMenuState::InternOnRun()
    {
        Gfx::MainMenu::OnRun();

        App::Application::ChangeState(CState::MainMenu);
    }
} // namespace App