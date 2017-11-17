
#include "app_droid/app_application.h"
#include "app_droid/app_main_menu_state.h"

#include "graphic/gfx_main_menu_state.h"

#include "gui/gui_main_menu_state.h"

#include "logic/lg_main_menu_state.h"

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
        BASE_CONSOLE_STREAMINFO("Enter main menu state.");

        Lg ::MainMenu::OnEnter();
        Gui::MainMenu::OnEnter();
        Gfx::MainMenu::OnEnter();
    }

    // -----------------------------------------------------------------------------

    void CMainMenuState::InternOnLeave()
    {
        Gfx::MainMenu::OnLeave();
        Gui::MainMenu::OnLeave();
        Lg ::MainMenu::OnLeave();

        BASE_CONSOLE_STREAMINFO("Leave main menu state.");
    }

    // -----------------------------------------------------------------------------

    void CMainMenuState::InternOnRun()
    {
        CState::EStateType NextState = CState::MainMenu;

        switch (Lg::MainMenu::OnRun())
        {
        case Lg::MainMenu::SResult::MainMenu:
            NextState = CState::MainMenu;
            break;
        case Lg::MainMenu::SResult::LoadMap:
            NextState = CState::LoadMap;
            break;
        }

        Gui::MainMenu::OnRun();
        Gfx::MainMenu::OnRun();

        App::Application::ChangeState(NextState);
    }
} // namespace App