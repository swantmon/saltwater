
#include "app/app_intro_state.h"

#include "base/base_console.h"

#include "graphic/gfx_intro_state.h"

#include "gui/gui_intro_state.h"

#include "logic/lg_intro_state.h"

namespace App
{
    CIntroState& CIntroState::GetInstance()
    {
        static CIntroState s_Singleton;
        
        return s_Singleton;
    }
} // namespace App

namespace App
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
        BASE_CONSOLE_STREAMINFO("App> Enter intro state.");

        Lg ::Intro::OnEnter();
        Gfx::Intro::OnEnter();
        Gui::Intro::OnEnter();
        
        return App::CState::Intro;
    }
    
    // -----------------------------------------------------------------------------
    
    CState::EStateType CIntroState::InternOnLeave()
    {
        Gui::Intro::OnLeave();
        Gfx::Intro::OnLeave();
        Lg ::Intro::OnLeave();

        BASE_CONSOLE_STREAMINFO("App> Leave intro state.");
        
        return App::CState::Intro;
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
} // namespace App