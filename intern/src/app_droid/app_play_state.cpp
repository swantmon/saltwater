
#include "app_droid/app_precompiled.h"

#include "app_droid/app_application.h"
#include "app_droid/app_play_state.h"

#include "camera/cam_control_manager.h"

#include "graphic/gfx_play_state.h"

#include "gui/gui_play_state.h"

#include "logic/lg_play_state.h"

namespace App
{
    CPlayState& CPlayState::GetInstance()
    {
        static CPlayState s_Singleton;

        return s_Singleton;
    }
} // namespace App

namespace App
{
    CPlayState::CPlayState()
    {

    }

    // -----------------------------------------------------------------------------

    CPlayState::~CPlayState()
    {

    }

    // -----------------------------------------------------------------------------

    void CPlayState::InternOnEnter()
    {
        // -----------------------------------------------------------------------------
        // Activate game control
        // -----------------------------------------------------------------------------
        Cam::ControlManager::SetActiveControl(Cam::CControl::GameControl);
    }

    // -----------------------------------------------------------------------------

    void CPlayState::InternOnLeave()
    {
    }

    // -----------------------------------------------------------------------------

    void CPlayState::InternOnRun()
    {
        App::Application::ChangeState(CState::Play);
    }
} // namespace App