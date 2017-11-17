
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
        BASE_CONSOLE_STREAMINFO("Enter play state.");

        // -----------------------------------------------------------------------------
        // Activate game control
        // -----------------------------------------------------------------------------
        Cam::ControlManager::SetActiveControl(Cam::CControl::GameControl);

        // -----------------------------------------------------------------------------
        // Running states
        // -----------------------------------------------------------------------------
        Lg ::Play::OnEnter();
        Gui::Play::OnEnter();
        Gfx::Play::OnEnter();
    }

    // -----------------------------------------------------------------------------

    void CPlayState::InternOnLeave()
    {
        Gfx::Play::OnLeave();
        Gui::Play::OnLeave();
        Lg ::Play::OnLeave();

        BASE_CONSOLE_STREAMINFO("Leave play state.");
    }

    // -----------------------------------------------------------------------------

    void CPlayState::InternOnRun()
    {
        CState::EStateType NextState = CState::Play;

        // -----------------------------------------------------------------------------
        // Update logic
        // -----------------------------------------------------------------------------
        switch (Lg::Play::OnRun())
        {
        case Lg::Play::SResult::Play:
            NextState = CState::Play;
            break;
        case Lg::Play::SResult::Exit:
            NextState = CState::Exit;
            break;
        }

        // -----------------------------------------------------------------------------
        // Update cameras and views depending on logic and world
        // -----------------------------------------------------------------------------
        Cam::ControlManager::Update();

        // -----------------------------------------------------------------------------
        // Update graphic & gui
        // -----------------------------------------------------------------------------
        Gui::Play::OnRun();
        Gfx::Play::OnRun();

        // -----------------------------------------------------------------------------
        // Change state
        // -----------------------------------------------------------------------------
        App::Application::ChangeState(NextState);
    }
} // namespace App