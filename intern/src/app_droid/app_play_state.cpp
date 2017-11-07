
#include "app_droid/app_application.h"
#include "app_droid/app_play_state.h"

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