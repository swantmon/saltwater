
#include "editor/edit_precompiled.h"

#include "base/base_console.h"

#include "camera/cam_control_manager.h"

#include "editor/edit_start_state.h"

#include "graphic/gfx_main.h"
#include "graphic/gfx_start_state.h"

#include "gui/gui_start_state.h"

#include "logic/lg_start_state.h"

namespace Edit
{
    CStartState& CStartState::GetInstance()
    {
        static CStartState s_Singleton;
        
        return s_Singleton;
    }
} // namespace Edit

namespace Edit
{
    CStartState::CStartState()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    CStartState::~CStartState()
    {
        
    }
    
    // -----------------------------------------------------------------------------
    
    CState::EStateType CStartState::InternOnEnter()
    {
        BASE_CONSOLE_STREAMINFO("Edit> Enter start state.");

        // -----------------------------------------------------------------------------
        // Start engine
        // -----------------------------------------------------------------------------
        Cam::ControlManager::CreateControl(Cam::CControl::EditorControl);
        Cam::ControlManager::CreateControl(Cam::CControl::GameControl);
        
        // -----------------------------------------------------------------------------
        // Start normal states
        // -----------------------------------------------------------------------------
        Lg ::Start::OnEnter();
        Gfx::Start::OnEnter();
        Gui::Start::OnEnter();
        
        return Edit::CState::Start;
    }
    
    // -----------------------------------------------------------------------------
    
    CState::EStateType CStartState::InternOnLeave()
    {
        Gui::Start::OnLeave();
        Gfx::Start::OnLeave();
        Lg ::Start::OnLeave();

        BASE_CONSOLE_STREAMINFO("Edit> Leave start state.");
        
        return Edit::CState::Start;
    }
    
    // -----------------------------------------------------------------------------
    
    CState::EStateType CStartState::InternOnRun()
    {
        CState::EStateType NextState = CState::Intro;
        
        Lg ::Start::OnRun();
        Gfx::Start::OnRun();
        Gui::Start::OnRun();
        
        return NextState;
    }
} // namespace Edit