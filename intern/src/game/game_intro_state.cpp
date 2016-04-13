
#include "base/base_console.h"

#include "game/game_intro_state.h"

#include "graphic/gfx_intro_state.h"

#include "gui/gui_intro_state.h"

#include "logic/lg_intro_state.h"

namespace Game
{
    CIntroState& CIntroState::GetInstance()
    {
        static CIntroState s_Singleton;
        
        return s_Singleton;
    }
} // namespace Game

namespace Game
{
    CIntroState::CIntroState()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    CIntroState::~CIntroState()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    CState::EStateType CIntroState::InternOnEnter()
    {
        BASE_CONSOLE_STREAMINFO("Game> Enter intro state.");

        Lg ::Intro::OnEnter();
        Gfx::Intro::OnEnter();
        Gui::Intro::OnEnter();
        
        return Game::CState::Intro;
    }
    
    // -----------------------------------------------------------------------------
    
    CState::EStateType CIntroState::InternOnLeave()
    {
        Gui::Intro::OnLeave();
        Gfx::Intro::OnLeave();
        Lg ::Intro::OnLeave();

        BASE_CONSOLE_STREAMINFO("Game> Leave intro state.");
        
        return Game::CState::Intro;
    }
    
    // -----------------------------------------------------------------------------
    
    CState::EStateType CIntroState::InternOnRun()
    {
        CState::EStateType NextState = CState::Intro;
        
        switch(Lg::Intro::OnRun())
        {
            case Lg::Intro::SResult::Intro:
                NextState = CState::Intro;
            break;
            case Lg::Intro::SResult::MainMenu:
                NextState = CState::MainMenu;
            break;
        }
        
        Gfx::Intro::OnRun();
        Gui::Intro::OnRun();
        
        return NextState;
    }
} // namespace Game