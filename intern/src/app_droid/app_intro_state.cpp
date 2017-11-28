
#include "app_droid/app_precompiled.h"

#include "app_droid/app_application.h"
#include "app_droid/app_intro_state.h"

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

    void CIntroState::InternOnEnter()
    {
        BASE_CONSOLE_STREAMINFO("Enter intro state.");

        Lg ::Intro::OnEnter();
        Gui::Intro::OnEnter();
        Gfx::Intro::OnEnter();
    }

    // -----------------------------------------------------------------------------

    void CIntroState::InternOnLeave()
    {
        Gfx::Intro::OnLeave();
        Gui::Intro::OnLeave();
        Lg ::Intro::OnLeave();

        BASE_CONSOLE_STREAMINFO("Leave intro state.");
    }

    // -----------------------------------------------------------------------------

    void CIntroState::InternOnRun()
    {
        CState::EStateType NextState = CState::Intro;

        switch (Lg::Intro::OnRun())
        {
        case Lg::Intro::SResult::Intro:
            NextState = CState::Intro;
            break;
        case Lg::Intro::SResult::MainMenu:
            NextState = CState::MainMenu;
            break;
        }

        Gui::Intro::OnRun();
        Gfx::Intro::OnRun();

        App::Application::ChangeState(NextState);
    }
} // namespace App