
#include "app/app_main_menu_state.h"

#include "base/base_console.h"

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
    
    CState::EStateType CMainMenuState::InternOnEnter()
    {
        BASE_CONSOLE_STREAMINFO("App> Enter main menu state.");

        Lg ::MainMenu::OnEnter();
        Gfx::MainMenu::OnEnter();
        Gui::MainMenu::OnEnter();
        
        return App::CState::MainMenu;
    }
    
    // -----------------------------------------------------------------------------
    
    CState::EStateType CMainMenuState::InternOnLeave()
    {
        Gui::MainMenu::OnLeave();
        Gfx::MainMenu::OnLeave();
        Lg ::MainMenu::OnLeave();

        BASE_CONSOLE_STREAMINFO("App> Leave main menu state.");
        
        return App::CState::MainMenu;
    }
    
    // -----------------------------------------------------------------------------
    
    CState::EStateType CMainMenuState::InternOnRun()
    {
        CState::EStateType NextState = CState::MainMenu;
        
        switch(Lg::MainMenu::OnRun())
        {
            case Lg::MainMenu::SResult::MainMenu:
                NextState = CState::MainMenu;
                break;
            case Lg::MainMenu::SResult::LoadMap:
                NextState = CState::LoadMap;
                break;
        }
        
        Gfx::MainMenu::OnRun();
        Gui::MainMenu::OnRun();
        
        return NextState;
    }
} // namespace App
