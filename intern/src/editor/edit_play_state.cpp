
#include "base/base_console.h"

#include "camera/cam_control_manager.h"

#include "editor/edit_play_state.h"

#include "graphic/gfx_play_state.h"

#include "gui/gui_play_state.h"

#include "logic/lg_play_state.h"

namespace Edit
{
    CPlayState& CPlayState::GetInstance()
    {
        static CPlayState s_Singleton;
        
        return s_Singleton;
    }
} // namespace Edit

namespace Edit
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
        BASE_CONSOLE_STREAMINFO("Edit> Enter play state.");

        Lg ::Play::OnEnter();
        Gui::Play::OnEnter();
        Gfx::Play::OnEnter();        
        
        return Edit::CState::Play;
    }
    
    // -----------------------------------------------------------------------------
    
    CState::EStateType CPlayState::InternOnLeave()
    {
        Gfx::Play::OnLeave();
        Gui::Play::OnLeave();
        Lg ::Play::OnLeave();

        BASE_CONSOLE_STREAMINFO("Edit> Leave play state.");
        
        return Edit::CState::Play;
    }
    
    // -----------------------------------------------------------------------------
    
    CState::EStateType CPlayState::InternOnRun()
    {
        CState::EStateType NextState = CState::Play;
        
        // -----------------------------------------------------------------------------
        // Update logic
        // -----------------------------------------------------------------------------
        Lg::Play::OnRun();
        
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
} // namespace Edit