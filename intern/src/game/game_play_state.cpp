
#include "game/game_precompiled.h"

#include "base/base_console.h"

#include "camera/cam_control_manager.h"

#include "game/game_play_state.h"

#include "graphic/gfx_play_state.h"

#include "gui/gui_play_state.h"

#include "logic/lg_play_state.h"

namespace Game
{
    CPlayState& CPlayState::GetInstance()
    {
        static CPlayState s_Singleton;
        
        return s_Singleton;
    }
} // namespace Game

namespace Game
{
    CPlayState::CPlayState()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    CPlayState::~CPlayState()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    CState::EStateType CPlayState::InternOnEnter()
    {
        BASE_CONSOLE_STREAMINFO("Game> Enter play state.");

        // -----------------------------------------------------------------------------
        // Activate game control
        // -----------------------------------------------------------------------------
        Cam::ControlManager::SetActiveControl(Cam::CControl::GameControl);

        // -----------------------------------------------------------------------------
        // Running states
        // -----------------------------------------------------------------------------
        Lg ::Play::OnEnter();
        Gui::Play::OnEnter();
        Gfx::Play::OnEnter();        
        
        return Game::CState::Play;
    }
    
    // -----------------------------------------------------------------------------
    
    CState::EStateType CPlayState::InternOnLeave()
    {
        Gfx::Play::OnLeave();
        Gui::Play::OnLeave();
        Lg ::Play::OnLeave();

        BASE_CONSOLE_STREAMINFO("Game> Leave play state.");
        
        return Game::CState::Play;
    }
    
    // -----------------------------------------------------------------------------
    
    CState::EStateType CPlayState::InternOnRun()
    {
        CState::EStateType NextState = CState::Play;
        
        // -----------------------------------------------------------------------------
        // Update logic
        // -----------------------------------------------------------------------------
        switch(Lg::Play::OnRun())
        {
            case Lg::Play::SResult::Play:
                NextState = CState::Play;
                break;
            case Lg::Play::SResult::Debug:
                NextState = CState::Debug;
                break;
            case Lg::Play::SResult::Exit:
                NextState = CState::Exit;
                break;
        }
        
        // -----------------------------------------------------------------------------
        // Update cameras and views depending on logic and world
        // -----------------------------------------------------------------------------
        Cam::ControlManager::Update();
        
        // -----------------------------------------------------------------------------
        // Update graphic and GUI
        // -----------------------------------------------------------------------------
        Gui::Play::OnRun();
        Gfx::Play::OnRun();
        
        return NextState;
    }
} // namespace Game