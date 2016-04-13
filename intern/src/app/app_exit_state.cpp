
#include "app/app_exit_state.h"

#include "base/base_console.h"

#include "graphic/gfx_exit_state.h"
#include "graphic/gfx_main.h"

#include "gui/gui_exit_state.h"

#include "logic/lg_exit_state.h"

namespace App
{
    CExitState& CExitState::GetInstance()
    {
        static CExitState s_Singleton;
        
        return s_Singleton;
    }
} // namespace App

namespace App
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
        BASE_CONSOLE_STREAMINFO("App> Enter exit state.");

        Lg ::Exit::OnEnter();
        Gfx::Exit::OnEnter();
        Gui::Exit::OnEnter();
        
        // -----------------------------------------------------------------------------
        // Stop engine
        // -----------------------------------------------------------------------------
        Gfx::Main::OnExit();
        
        return App::CState::Exit;
    }
    
    // -----------------------------------------------------------------------------
    
    CState::EStateType CExitState::InternOnLeave()
    {
        Gui::Exit::OnLeave();
        Gfx::Exit::OnLeave();
        Lg ::Exit::OnLeave();

        BASE_CONSOLE_STREAMINFO("App> Leave exit state.");
        
        return App::CState::Exit;
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
} // namespace App