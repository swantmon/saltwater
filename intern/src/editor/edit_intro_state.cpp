
#include "base/base_console.h"

#include "editor/edit_intro_state.h"

#include "graphic/gfx_intro_state.h"

#include "gui/gui_intro_state.h"

#include "logic/lg_intro_state.h"

namespace Edit
{
    CIntroState& CIntroState::GetInstance()
    {
        static CIntroState s_Singleton;
        
        return s_Singleton;
    }
} // namespace Edit

namespace Edit
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
        BASE_CONSOLE_STREAMINFO("Edit> Enter intro state.");

        Lg ::Intro::OnEnter();
        Gfx::Intro::OnEnter();
        Gui::Intro::OnEnter();
        
        return Edit::CState::Intro;
    }
    
    // -----------------------------------------------------------------------------
    
    CState::EStateType CIntroState::InternOnLeave()
    {
        Gui::Intro::OnLeave();
        Gfx::Intro::OnLeave();
        Lg ::Intro::OnLeave();

        BASE_CONSOLE_STREAMINFO("Edit> Leave intro state.");
        
        return Edit::CState::Intro;
    }
    
    // -----------------------------------------------------------------------------
    
    CState::EStateType CIntroState::InternOnRun()
    {
        CState::EStateType NextState = CState::Edit;
        
        Lg ::Intro::OnRun();
        Gfx::Intro::OnRun();
        Gui::Intro::OnRun();
        
        return NextState;
    }
} // namespace Edit