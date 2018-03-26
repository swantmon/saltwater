
#include "app_droid/app_precompiled.h"

#include "app_droid/app_application.h"
#include "app_droid/app_start_state.h"

#include "camera/cam_control_manager.h"

#include "core/core_jni_interface.h"

#include "graphic/gfx_start_state.h"

#include "gui/gui_start_state.h"

#include "logic/lg_start_state.h"

#include "mr/mr_control_manager.h"

namespace App
{
    CStartState& CStartState::GetInstance()
    {
        static CStartState s_Singleton;

        return s_Singleton;
    }
} // namespace App

namespace App
{
    CStartState::CStartState()
    {

    }

    // -----------------------------------------------------------------------------

    CStartState::~CStartState()
    {

    }

    // -----------------------------------------------------------------------------

    void CStartState::InternOnEnter()
    {
        BASE_CONSOLE_STREAMINFO("Enter start state.");

        // -----------------------------------------------------------------------------
        // Start engine
        // -----------------------------------------------------------------------------
        Cam::ControlManager::CreateControl(Cam::CControl::GameControl);

        // -----------------------------------------------------------------------------
        // Start normal states
        // -----------------------------------------------------------------------------
        Lg ::Start::OnEnter();
        Gui::Start::OnEnter();
        Gfx::Start::OnEnter();

        // -----------------------------------------------------------------------------
        // Setup mixed reality
        // -----------------------------------------------------------------------------
        MR::ControlManager::OnStart();
    }

    // -----------------------------------------------------------------------------

    void CStartState::InternOnLeave()
    {
        Gfx::Start::OnLeave();
        Gui::Start::OnLeave();
        Lg ::Start::OnLeave();

        BASE_CONSOLE_STREAMINFO("Leave start state.");
    }

    // -----------------------------------------------------------------------------

    void CStartState::InternOnRun()
    {
        CState::EStateType NextState = CState::Intro;

        switch (Lg::Start::OnRun())
        {
        case Lg::Start::SResult::Start:
            NextState = CState::Start;
            break;
        case Lg::Start::SResult::Intro:
            NextState = CState::Intro;
            break;
        }

        Gui::Start::OnRun();
        Gfx::Start::OnRun();

        App::Application::ChangeState(NextState);
    }
} // namespace App