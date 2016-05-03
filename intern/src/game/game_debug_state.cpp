
#include "game/game_precompiled.h"

#include "base/base_console.h"

#include "game/game_debug_state.h"

#include "camera/cam_control_manager.h"

#include "graphic/gfx_debug_state.h"

#include "gui/gui_debug_state.h"

#include "logic/lg_debug_state.h"

namespace Game
{
    CDebugState& CDebugState::GetInstance()
    {
        static CDebugState s_Singleton;
        
        return s_Singleton;
    }
} // namespace Game

namespace Game
{
    CDebugState::CDebugState()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    CDebugState::~CDebugState()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    CState::EStateType CDebugState::InternOnEnter()
    {
        BASE_CONSOLE_STREAMINFO("Game> Enter debug state.");

        Lg ::Debug::OnEnter();
        Gfx::Debug::OnEnter();
        Gui::Debug::OnEnter();
        
        return Game::CState::Debug;
    }
    
    // -----------------------------------------------------------------------------
    
    CState::EStateType CDebugState::InternOnLeave()
    {
        Gui::Debug::OnLeave();
        Gfx::Debug::OnLeave();
        Lg ::Debug::OnLeave();

        BASE_CONSOLE_STREAMINFO("Game> Leave debug state.");
        
        return Game::CState::Debug;
    }
    
    // -----------------------------------------------------------------------------
    
    CState::EStateType CDebugState::InternOnRun()
    {
        CState::EStateType NextState = CState::Debug;
        
        // -----------------------------------------------------------------------------
        // Update camera with cameras and views (stuff for graphic and logic)
        // -----------------------------------------------------------------------------
        Cam::ControlManager::Update();
        
        // -----------------------------------------------------------------------------
        // Update game
        // -----------------------------------------------------------------------------
        switch(Lg::Debug::OnRun())
        {
            case Lg::Debug::SResult::Debug:
                NextState = CState::Debug;
                break;
            case Lg::Debug::SResult::Play:
                NextState = CState::Play;
                break;
        }
        
        Gfx::Debug::OnRun();
        Gui::Debug::OnRun();
        
        return NextState;
    }
} // namespace Game