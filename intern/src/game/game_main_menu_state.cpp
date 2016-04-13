
#include "base/base_console.h"

#include "game/game_main_menu_state.h"

#include "graphic/gfx_main_menu_state.h"

#include "gui/gui_main_menu_state.h"

#include "logic/lg_main_menu_state.h"

namespace Game
{
    CMainMenuState& CMainMenuState::GetInstance()
    {
        static CMainMenuState s_Singleton;
        
        return s_Singleton;
    }
} // namespace Game

namespace Game
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
        BASE_CONSOLE_STREAMINFO("Game> Enter main menu state.");

        Lg ::MainMenu::OnEnter();
        Gfx::MainMenu::OnEnter();
        Gui::MainMenu::OnEnter();
        
        return Game::CState::MainMenu;
    }
    
    // -----------------------------------------------------------------------------
    
    CState::EStateType CMainMenuState::InternOnLeave()
    {
        Gui::MainMenu::OnLeave();
        Gfx::MainMenu::OnLeave();
        Lg ::MainMenu::OnLeave();

        BASE_CONSOLE_STREAMINFO("Game> Leave main menu state.");
        
        return Game::CState::MainMenu;
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
} // namespace Game
