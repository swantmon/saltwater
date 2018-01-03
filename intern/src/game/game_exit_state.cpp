
#include "game/game_precompiled.h"

#include "base/base_console.h"

#include "game/game_exit_state.h"

#include "graphic/gfx_exit_state.h"
#include "graphic/gfx_main.h"

#include "gui/gui_exit_state.h"

#include "logic/lg_exit_state.h"

namespace Game
{
    CExitState& CExitState::GetInstance()
    {
        static CExitState s_Singleton;
        
        return s_Singleton;
    }
} // namespace Game

namespace Game
{
    CExitState::CExitState()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    CExitState::~CExitState()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    CState::EStateType CExitState::InternOnEnter()
    {
        BASE_CONSOLE_STREAMINFO("Game> Enter exit state.");

        Lg ::Exit::OnEnter();
        Gfx::Exit::OnEnter();
        Gui::Exit::OnEnter();

        return Game::CState::Exit;
    }
    
    // -----------------------------------------------------------------------------
    
    CState::EStateType CExitState::InternOnLeave()
    {
        Gui::Exit::OnLeave();
        Gfx::Exit::OnLeave();
        Lg ::Exit::OnLeave();

        BASE_CONSOLE_STREAMINFO("Game> Leave exit state.");
        
        return Game::CState::Exit;
    }
    
    // -----------------------------------------------------------------------------
    
    CState::EStateType CExitState::InternOnRun()
    {
        CState::EStateType NextState = CState::Exit;
        
        switch(Lg::Exit::OnRun())
        {
            case Lg::Exit::SResult::Exit:
                NextState = CState::Exit;
            break;
        }
        
        Gfx::Exit::OnRun();
        Gui::Exit::OnRun();
        
        return NextState;
    }
} // namespace Game