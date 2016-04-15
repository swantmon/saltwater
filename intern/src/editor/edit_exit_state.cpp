
#include "base/base_console.h"

#include "editor/edit_exit_state.h"

#include "graphic/gfx_exit_state.h"
#include "graphic/gfx_main.h"

#include "gui/gui_exit_state.h"

#include "logic/lg_exit_state.h"

namespace Edit
{
    CExitState& CExitState::GetInstance()
    {
        static CExitState s_Singleton;
        
        return s_Singleton;
    }
} // namespace Edit

namespace Edit
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
        BASE_CONSOLE_STREAMINFO("Edit> Enter exit state.");

        Lg ::Exit::OnEnter();
        Gfx::Exit::OnEnter();
        Gui::Exit::OnEnter();
        
        // -----------------------------------------------------------------------------
        // Stop engine
        // -----------------------------------------------------------------------------
        Gfx::Main::OnExit();
        
        return Edit::CState::Exit;
    }
    
    // -----------------------------------------------------------------------------
    
    CState::EStateType CExitState::InternOnLeave()
    {
        Gui::Exit::OnLeave();
        Gfx::Exit::OnLeave();
        Lg ::Exit::OnLeave();

        BASE_CONSOLE_STREAMINFO("Edit> Leave exit state.");
        
        return Edit::CState::Exit;
    }
    
    // -----------------------------------------------------------------------------
    
    CState::EStateType CExitState::InternOnRun()
    {
        CState::EStateType NextState = CState::Exit;
        
        Lg ::Exit::OnRun();
        Gfx::Exit::OnRun();
        Gui::Exit::OnRun();
        
        return NextState;
    }
} // namespace Edit