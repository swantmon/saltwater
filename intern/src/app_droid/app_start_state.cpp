
#include "app_droid/app_precompiled.h"

#include "app_droid/app_application.h"
#include "app_droid/app_start_state.h"

#include "engine/camera/cam_control_manager.h"

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
        // -----------------------------------------------------------------------------
        // Prepare controls
        // -----------------------------------------------------------------------------
        Cam::ControlManager::CreateControl(Cam::CControl::GameControl);
    }

    // -----------------------------------------------------------------------------

    void CStartState::InternOnLeave()
    {
    }

    // -----------------------------------------------------------------------------

    void CStartState::InternOnRun()
    {
        App::Application::ChangeState(CState::Intro);
    }
} // namespace App